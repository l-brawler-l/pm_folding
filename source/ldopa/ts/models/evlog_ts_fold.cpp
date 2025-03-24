#include "xi/ldopa/ts/models/evlog_ts_fold.h"
#include <numeric> // std::gcd



namespace xi { namespace ldopa { namespace ts {

//==============================================================================
// class EvLogTSWithParVecs
//==============================================================================

EvLogTSWithParVecs::EvLogTSWithParVecs(IStateIDsPool* stIDsPool)
    : Base(stIDsPool)
    , _settings(DEF_SETTINGS)
    , _diffs(Matrix())
{
    _stateParVec.insert(std::make_pair(getInitState(), ParikhVector()));
}


//------------------------------------------------------------------------------


EvLogTSWithParVecs::EvLogTSWithParVecs(IStateIDsPool* stIDsPool, 
    const AttrIndexMap& attrInds)
    : Base(stIDsPool)
    , _settings(DEF_SETTINGS)
    , _attrInds(attrInds)
    , _diffs(Matrix())
{
    _stateParVec.insert(std::make_pair(getInitState(), ParikhVector()));
}


//------------------------------------------------------------------------------

EvLogTSWithParVecs::EvLogTSWithParVecs(const EvLogTSWithParVecs& that)
    : Base(that)
    , _settings(that._settings)
    , _attrInds(that._attrInds)
    , _diffs(that._diffs)
{
    validateStates(that);
}

//------------------------------------------------------------------------------

// EvLogTSWithParVecs& EvLogTSWithParVecs::operator=(const EvLogTSWithParVecs& that)
// {
//     EvLogTSWithParVecs tmp(that);
//     std::swap(tmp, *this);

//     return *this;
// }

//------------------------------------------------------------------------------

void EvLogTSWithParVecs::validateStates(const EvLogTSWithParVecs& that)
{
    LogTS::StateIter stOrig, stOrigEnd, stCur, stCurEnd;
    boost::tie(stOrig, stOrigEnd) = that.getStates();
    boost::tie(stCur, stCurEnd) = getStates();

    // итерируем по вершинам исходного графа
    for (; stOrig != stOrigEnd; ++stOrig, ++stCur)
    {        
        auto it = that._stateParVec.find(*stOrig);
        if (it != that._stateParVec.end())                // если ключ для исходного вертекса
            // _stateParVec[*stCur] = it->second;
            _stateParVec.insert(std::make_pair(*stCur, it->second));

    }
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::~EvLogTSWithParVecs()
{
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::Index EvLogTSWithParVecs::GetOrAddAttrIndex(const Attribute& lbl) 
{
    auto it = _attrInds.find(lbl);
    if (it == _attrInds.end()) {
        Index new_ind = _attrInds.size();
        _attrInds[lbl] = new_ind;
        return new_ind;

    }
    return (*it).second;
}

EvLogTSWithParVecs::IndexRes EvLogTSWithParVecs::GetAttrIndex(const Attribute& lbl) const 
{
    auto it = _attrInds.find(lbl);
    if (it == _attrInds.end()) {
        return std::make_pair(0, false);
    }
    return std::make_pair((*it).second, true);
}

//------------------------------------------------------------------------------


EvLogTSWithParVecs::Transition EvLogTSWithParVecs::getOrAddTrans(State s, 
    State t, const Attribute& lbl) 
{ 
    GetOrAddAttrIndex(lbl);
    return _ts->getOrAddTrans(s, t, lbl); 
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::ParikhVectorRes EvLogTSWithParVecs::getParikhVector(State s) const
{
    StateParikhVectorMap::const_iterator pv = _stateParVec.find(s);
    if (pv == _stateParVec.end())
        return std::make_pair(ParikhVector(), false);   
    
    ParikhVector pv_ans = (*pv).second;
    return std::make_pair(pv_ans, true);
}

EvLogTSWithParVecs::ParikhVectorPtr EvLogTSWithParVecs::getParikhVectorPtr(State s)
{
    StateParikhVectorMap::iterator pv = _stateParVec.find(s);
    if (pv == _stateParVec.end())
        return nullptr;
        
    return &((*pv).second);
}

//------------------------------------------------------------------------------

void EvLogTSWithParVecs::setParikhVector(State s, const ParikhVector* pv)
{
    StateParikhVectorMap::iterator pvIt = _stateParVec.find(s);
    if (pvIt == _stateParVec.end())
        _stateParVec.insert(std::make_pair(s, *pv));
    else
        _stateParVec[s] = *pv;
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::Value EvLogTSWithParVecs::getStateAttrCnt(State s, const Attribute& lbl) const
{
    StateParikhVectorMap::const_iterator pv_it = _stateParVec.find(s);
    if (pv_it == _stateParVec.end()) {
        return 0;
    }
    IndexRes index_res = GetAttrIndex(lbl);
    if (!index_res.second) {
        return 0;
    }
    return (*pv_it).second.GetAttrCnt(index_res.first);
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::Transition EvLogTSWithParVecs::getOrAddTransPV(State s, State t, 
    const Attribute& lbl, Value lblCnt) 
{
    Index index = GetOrAddAttrIndex(lbl);

    ParikhVectorPtr pv_s = getParikhVectorPtr(s);
    if (!pv_s) {
        throw LdopaException("Parikh vector for previous state does not exist");
    }

    Transition tr = Base::getOrAddTrans(s, t, lbl);

    ParikhVectorPtr pv_t = getParikhVectorPtr(t);
    if (!pv_t) {
        setParikhVector(t, pv_s);

        pv_t = getParikhVectorPtr(t);
        (*pv_t).AddAttrCnt(index, lblCnt);
        
    } else if (!_settings[SET_IGNORE_DIFFS]) {
        ParikhVector pv = GetDiff(*pv_t, *pv_s);
        pv.AddAttrCnt(index, lblCnt);
        _diffs.PushBack(pv);
    }
    return tr;
}

//------------------------------------------------------------------------------

}}} // namespace xi { namespace ldopa { namespace ts {
