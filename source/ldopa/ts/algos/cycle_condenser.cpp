#include "xi/ldopa/ts/algos/cycle_condenser.h"
#include <numeric>

namespace xi { namespace ldopa { namespace ts {   //


//==============================================================================
// class ParikhTrie
//==============================================================================

ParikhTrie::ParikhTrie(TS* ts) : _ts(ts)
{
    _initSt = new State();
    *_initSt = addState();
    _stPV = new StateToPVMap();
    _initDom = new Domain();
    _maxv = 0;
}

//------------------------------------------------------------------------------

ParikhTrie::~ParikhTrie() 
{ 
    delete _initSt;
    delete _stPV;
    delete _initDom;
    BaseGraph::deleteGraph(); 
}

//------------------------------------------------------------------------------

ParikhVector* ParikhTrie::extractParikhVector(State s) {
    StateToPVMap::const_iterator pv_ptr_it = _stPV->find(s);
    if (pv_ptr_it == _stPV->end()) {
        return nullptr;
    }
    return (*pv_ptr_it).second;
}

//------------------------------------------------------------------------------

ParikhTrie::State ParikhTrie::addState(ParikhVector* pv)
{
    State v = BaseGraph::addVertex();
    if (pv) {
        _stPV->insert({v, pv});
    }
    return v;
}

//------------------------------------------------------------------------------

ParikhTrie::TransRes ParikhTrie::getTrans(State s, Value lbl)
{
    OtransIter tCur, tEnd;
    boost::tie(tCur, tEnd) = getOutTransitions(s);
    size_t chCur = 0, chEnd = BaseGraph::getOutEdgesNum(s);
    // ужасный костыль, но без него tCur почему-то не оказывается
    // равным tEnd и вылезает ошибка.
    for (; tCur != tEnd && chCur < chEnd; ++tCur, ++chCur)
    {
        Value cLbl = extractTransIndex(*tCur);    
        if (cLbl == lbl)
        {
            return std::make_pair(*tCur, true);
        }
    }
    return std::make_pair(Transition(), false);
}

//------------------------------------------------------------------------------

ParikhTrie::StateTrans ParikhTrie::getOrAddTargetState(State s, Value lbl, ParikhVector* pv)
{
    // сперва ищем существующую
    TransRes tr = getTrans(s, lbl);
    if (tr.second)                      // есть такая
        return std::make_pair(getTargState(tr.first), tr.first);

    // иначе — добавляем новую
    State t = addState(pv);
    Transition new_tr = addTransitionInternal(s, t, lbl);

    return std::make_pair(t, new_tr);
}

//------------------------------------------------------------------------------

void ParikhTrie::build() {
    
    if (!_ts)
        // throw LdopaException("Can't build a Parikh Vector: no event log is set.");
        return;

    TS::StateIter sCur, sEnd;
    for(boost::tie(sCur, sEnd) = _ts->getStates(); sCur != sEnd; ++sCur) {
        processParikhVector(*sCur);
    }
}

//------------------------------------------------------------------------------

void ParikhTrie::processParikhVector(TS::State ts_s) {
    State src = *_initSt;

    ParikhVector* pv = _ts->getParikhVectorPtr(ts_s);
    size_t max_cnt = _ts->getAttributeNum();

    if (max_cnt == 0) {
        return;
    }

    ParikhVector* arg = nullptr;

    for(size_t i = 0; i < max_cnt; ++i) {
        Value attr_cnt = pv->GetAttrCnt(i);
        _maxv = std::max(_maxv, attr_cnt);

        if (i + 1 == max_cnt) {
            arg = pv;
        }
        StateTrans st_tr = getOrAddTargetState(src, attr_cnt, arg);
        src = st_tr.first;

        if (i == 0) {
            auto dom_it = _initDom->find(attr_cnt);
            if (dom_it == _initDom->end()) {
                _initDom->insert(std::make_pair(attr_cnt, st_tr.second));
            }
        }
    }
}

//------------------------------------------------------------------------------

void ParikhTrie::TandemSearch(Matrix& pvDiffs, unsigned int k)
{
    unsigned int maxv = static_cast<unsigned int>(_maxv);
    std::vector<bool> used(maxv + 1, false);

    for (unsigned int i = k + 1; i <= maxv; ++i) {
        if (used[i]) {
            continue;
        }
        unsigned int m = 0;
        while (m <= maxv) {
            for (unsigned int j = 0; j + m <= _initDom->size(); ++j) {
                auto it1 = _initDom->find(static_cast<int>(j));
                auto it2 = _initDom->find(static_cast<int>(j + m));
                auto it_end = _initDom->end();

                if (it1 == it_end || it2 == it_end) {
                    continue;
                }

                ParikhTrie::State v1 = getTargState((*it1).second);

                if (m == 0) {
                    Recur(pvDiffs, v1, i);
                } else {
                    ParikhTrie::State v2 = getTargState((*it2).second);
                    Recur(pvDiffs, v1, v2, i);
                }
                
            }
            used[m] = true;
            m += i;
        } 
    }
}

//------------------------------------------------------------------------------

void ParikhTrie::Recur(Matrix& pvDiffs, ParikhTrie::State v1, ParikhTrie::State v2, unsigned int i) 
{
    ///TODO: отдельный случай, когда вершины равны
    ParikhVector* pv1 = extractParikhVector(v1);
    ParikhVector* pv2 = extractParikhVector(v2);
    if (pv1 && pv2) {  // если зашли в "листья" графа
        pvDiffs.PushBack(GetDiff(*pv1, *pv2));
        return;
    }
    // иначе
    ParikhTrie::OtransIter tr1Cur, tr1End, tr2Cur, tr2End;
    boost::tie(tr1Cur, tr1End) = getOutTransitions(v1);
    for (; tr1Cur != tr1End; ++tr1Cur) {
        Value index1 = extractTransIndex(*tr1Cur);

        boost::tie(tr2Cur, tr2End) = getOutTransitions(v2);
        for (; tr2Cur != tr2End; ++tr2Cur) {
            Value index2 = extractTransIndex(*tr2Cur);
            
            if (static_cast<unsigned int>(abs(index1 - index2)) % i == 0) {
                ParikhTrie::State new_v1 = getTargState(*tr1Cur);
                ParikhTrie::State new_v2 = getTargState(*tr2Cur);
                Recur(pvDiffs, new_v1, new_v2, i);
            }
        }
    }
}

//------------------------------------------------------------------------------

void ParikhTrie::Recur(Matrix& pvDiffs, ParikhTrie::State v, unsigned int i) 
{
    ///TODO: отдельный случай, когда вершины равны
    ParikhVector* pv = extractParikhVector(v);
    if (pv) {  // если зашли в "листья" графа
        // то выходим, нам неинтересны нулевые векторы
        return;
    }
    // иначе
    ParikhTrie::OtransIter tr1Cur, trEnd, tr2Cur;
    boost::tie(tr1Cur, trEnd) = getOutTransitions(v);
    for (; tr1Cur != trEnd; ++tr1Cur) {
        Value index1 = extractTransIndex(*tr1Cur);

        tr2Cur = tr1Cur;
        ParikhTrie::State new_v = getTargState(*tr1Cur);
        Recur(pvDiffs, new_v, i);

        ++tr2Cur;

        for (; tr2Cur != trEnd; ++tr2Cur) {
            Value index2 = extractTransIndex(*tr2Cur);
            
            if (static_cast<unsigned int>(abs(index1 - index2)) % i == 0) {
                ParikhTrie::State new_v1 = getTargState(*tr1Cur);
                ParikhTrie::State new_v2 = getTargState(*tr2Cur);
                Recur(pvDiffs, new_v1, new_v2, i);
            }
        }
    }
}







//==============================================================================
// class CycleCondensedTsBuilder
//==============================================================================

CycleCondensedTsBuilder::CycleCondensedTsBuilder(TS* ts, 
    CondensedStateFunc* sf, FixedIntListStateIdsPool* stIDsPool)
    : _srcTs(ts)
    , _sf(sf)
    , _stIDsPool(stIDsPool)
    , _ts(nullptr)
    , _k(0)
{

}

//------------------------------------------------------------------------------

CycleCondensedTsBuilder::~CycleCondensedTsBuilder()
{
    // если TS не была отдетачена, значит тот билдер управляет временем жизни ее
    cleanTS();
}

//------------------------------------------------------------------------------

void CycleCondensedTsBuilder::cleanTS()
{
    if (_ts)
    {
        delete _ts;
        _ts = nullptr;
    }
}


//------------------------------------------------------------------------------

CycleCondensedTsBuilder::TS* CycleCondensedTsBuilder::detach()
{
    if (!_ts)
        return nullptr;

    TS* ts = _ts;
    _ts = nullptr;

    return ts;
}

//------------------------------------------------------------------------------

CycleCondensedTsBuilder::TS* CycleCondensedTsBuilder::build(unsigned int k)
{
    cleanTS();

    if (!_srcTs)
        throw LdopaException("Can't build a TS: no source TS is set.");
    if (!_sf)
        throw LdopaException("Can't build a TS: no state function is set.");
    if (!_stIDsPool)
        throw LdopaException("Can't build a TS: no state IDs pool is set.");
   

    _k = k;

    _sf->reset();
    

    XI_LDOPA_ELAPSEDTIME_START(timer)


    // Создаем префиксное дерево Париха
    ParikhTrie ptrie(_srcTs); 
    ptrie.build();

    // Находим все разницы в векторах Париха, которые нужно "занулить"
    Matrix pvDiffs(_srcTs->getAttributeNum());
    ptrie.TandemSearch(pvDiffs, _k);

    // Находим базис регионов
    Matrix basis = pvDiffs.GetNullspace();

    // Добавляем его в функцию, по которой будем строить новый TS
    _sf->setBasis(basis);
    _stIDsPool->setLimit(basis.getParikhVectors().size());

    _ts = new TS(_stIDsPool, _srcTs->getMapOfAttrsToIndexes());

    // Копируем старый TS, применяя к состояниям функцию _sf.
    copyTS();


    XI_LDOPA_ELAPSEDTIME_STOP(timer)

    return _ts;
}


//------------------------------------------------------------------------------

void CycleCondensedTsBuilder::copyTS()
{
    std::queue<std::pair<TS::State, TS::State>> q;

    TS::State origInit = _srcTs->getInitState();
    TS::State curInit = copyState(origInit);
    // TS::State curInit = _ts->getInitState();

    q.push(std::make_pair(curInit, origInit));

    while (!q.empty()) {

        TS::State cur, orig;
        boost::tie(cur, orig) = q.front();
        q.pop();

        TS::OtransIter origBeg, origEnd;
        boost::tie(origBeg, origEnd) = _srcTs->getOutTransitions(orig);

        for (; origBeg != origEnd; ++origBeg) {
            const TS::Attribute& lbl = _srcTs->getTransLbl(*origBeg);

            TS::State origTrg = _srcTs->getTargState(*origBeg);
            TS::State curTrg = copyState(origTrg);

            TS::Transition t = _ts->getOrAddTrans(cur, curTrg, lbl);

            q.push(std::make_pair(curTrg, origTrg));
        }
    }
}

//------------------------------------------------------------------------------

CycleCondensedTsBuilder::TS::State CycleCondensedTsBuilder::copyState(
    TS::State stOrig) 
{
    const TS::ParikhVectorRes& pv_res = _srcTs->getParikhVector(stOrig);
    if (!pv_res.second) {
        throw LdopaException("State in TS does not have parikh vector.");
    }

    const IStateId* stID = _sf->makeState(pv_res.first);
    TS::State s = _ts->getOrAddState(stID);

    return s;
}


}}} // namespace xi { namespace ldopa { namespace ts {