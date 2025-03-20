#include "xi/ldopa/ts/algos/cycle_condenser.h"
#include <numeric>

namespace xi { namespace ldopa { namespace ts {   //

//==============================================================================
//  class Matrix
//==============================================================================

Matrix::Matrix(size_t maxSize)
    : _maxSize(maxSize)
{

}

//------------------------------------------------------------------------------

Matrix::~Matrix()
{

}

//------------------------------------------------------------------------------

Matrix::Matrix(const Matrix& rhs)
    : _matrix(rhs._matrix)
{

}

//------------------------------------------------------------------------------

Matrix& Matrix::operator=(const Matrix& rhs)
{
    Matrix tmp(rhs);
    std::swap(tmp, *this);

    return *this;
}

//------------------------------------------------------------------------------

void Matrix::PushBack(const ParikhVector& pv)
{
    _matrix.push_back(pv);
    // _matrix.back().Resize(_maxSize);
}

//------------------------------------------------------------------------------

Matrix::Value Matrix::GetElem(Index i, Index j)
{
    return _matrix[i].GetAttrCnt(j);
}

//------------------------------------------------------------------------------

Matrix::IndexVector Matrix::GaussianElimination()
{
    size_t N = _matrix.size();
    size_t M = _maxSize;
    size_t cur_row = 0;

    IndexVector ans;
    for (size_t k = 0; k < M; ++k)
    {
        if (cur_row >= N) {
            ans.push_back(k);
            continue;
        }

        size_t i_max = cur_row;
        Value v_max = GetElem(i_max, k);

        for (size_t i = cur_row + 1; i < N; ++i) 
        {
            if (abs(GetElem(i, k)) > v_max) {
                v_max = GetElem(i, k);
                i_max = i;
            }
        }

        if (v_max == 0) {
            ans.push_back(k);
            continue;
        }

        if (i_max != cur_row) {
            std::swap(_matrix[i_max], _matrix[cur_row]);
        }

        for (size_t i = 0; i < N; ++i) 
        {
            if (i == cur_row) {
                continue;
            }
            _matrix[i].SubstractSuffix(k, _matrix[cur_row]);
        }
        ++cur_row;
    }

    return ans;
}

//------------------------------------------------------------------------------

Matrix Matrix::GetNullspace()
{
    IndexVector freeVars = GaussianElimination();

    IndexVector pivotVars;
    pivotVars.reserve(_maxSize - freeVars.size());

    size_t fv_ind = 0; // индекс по вектору свободных переменных

    Matrix nullspace(_maxSize);
    nullspace._matrix.reserve(freeVars.size()); // мы знаем, что в базисе будет столько элементов

    for (size_t i = 0; i < _matrix.size() && fv_ind < freeVars.size(); ++i) {
        if (i != freeVars[fv_ind]) { // если главная переменная:
            pivotVars.push_back(i); // то сохраняем в отдельный массив
            continue;
        }
        Value factor = 1; 
        // считаем наименьшее такое число, чтобы в итоговом векторе
        // не было дробных чисел.
        for (size_t j = 0; j < pivotVars.size(); ++j) {
            Value free = GetElem(j, i);
            Value pivot = GetElem(j, pivotVars[j]);

            Value j_factor = std::lcm(free, pivot) / free;
            factor = std::lcm(factor, j_factor);
        }
        ParikhVector solution;
        solution.AddAttrCnt(i, factor);
        // считаем сам вектор
        for (size_t j = 0; j < pivotVars.size(); ++j) {
            Value free = GetElem(j, i);
            Value pivot = GetElem(j, pivotVars[j]);

            Value coef = (free * factor) / pivot;
            solution.AddAttrCnt(pivotVars[j], coef);
        }
        // добавляем в ответ
        nullspace.PushBack(solution);
        // смотрим на следующий индекс свободной переменной
        ++fv_ind;

    }
    return nullspace;
}

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
    build();
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
    for (boost::tie(tCur, tEnd) = getOutTransitions(s); tCur != tEnd; ++tCur)
    {
        Value cLbl = extractTransLbl(*tCur);    
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
    State src = _initSt;

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
            if (dom_it != _initDom->end()) {
                (*dom_it).second = st_tr.second;
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
                ParikhTrie::State v2 = getTargState((*it2).second);
                Recur(pvDiffs, v1, v2, i);
            }
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
    }
    // иначе
    ParikhTrie::OtransIter tr1Cur, tr1End, tr2Cur, tr2End;
    boost::tie(tr1Cur, tr1End) = getOutTransitions(v1);
    boost::tie(tr2Cur, tr2End) = getOutTransitions(v2);
    for (; tr1Cur != tr1End; ++tr1Cur) {
        Value index1 = extractTransLbl(*tr1Cur);

        for (; tr2Cur != tr2End; ++tr2Cur) {
            Value index2 = extractTransLbl(*tr2Cur);
            
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

CycleCondensedTsBuilder::CycleCondensedTsBuilder(TS* ts)
    : _srcTs(ts)
    , _ts(nullptr)
    , _k(0)
    , _ptrie(nullptr)
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

    if (_ptrie)
    {
        delete _ptrie;
        _ptrie = nullptr;
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
    if (!_srcTs)
        throw LdopaException("No source (full) TS set.");

    _k = k;

    // очищаем предыдущую TS, если была
    cleanTS();

    XI_LDOPA_ELAPSEDTIME_START(timer)

    _ts = new TS(*_srcTs);                                  // создаем точную копию пока
    _ptrie = new ParikhTrie(_srcTs);
    _ptrie->build();
    Matrix pvDiffs(_srcTs->getAttributeNum());
    _ptrie->TandemSearch(pvDiffs, _k);
    Matrix basis = pvDiffs.GetNullspace();



    XI_LDOPA_ELAPSEDTIME_STOP(timer)

    return _ts;
}


//------------------------------------------------------------------------------


}}} // namespace xi { namespace ldopa { namespace ts {