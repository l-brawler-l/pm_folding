#include "xi/ldopa/ts/models/evlog_ts_fold.h"
#include <numeric> // std::gcd



namespace xi { namespace ldopa { namespace ts {


//==============================================================================
// class ParikhVector
//==============================================================================

ParikhVector::ParikhVector() 
{

}

//------------------------------------------------------------------------------

ParikhVector::~ParikhVector() 
{

}

//------------------------------------------------------------------------------

ParikhVector::ParikhVector(const ParikhVector& that) 
    : _v(that._v)
{

}

//------------------------------------------------------------------------------

ParikhVector& ParikhVector::operator=(const ParikhVector& that)
{
    ParikhVector tmp(that);
    std::swap(tmp, *this);

    return *this;
}

//------------------------------------------------------------------------------

ParikhVector GetDiff(ParikhVector lhs, const ParikhVector& rhs)
{
    lhs.Resize(rhs._v.size());
    for (ParikhVector::Index i = 0; i < rhs._v.size(); ++i) {
        lhs._v[i] -= rhs._v[i];
    }
    return lhs;
}

//------------------------------------------------------------------------------

void ParikhVector::SubstractSuffix(Index k, const ParikhVector& rhs)
{
    Resize(rhs._v.size());

    Value lcm = std::lcm(_v[k], rhs._v[k]);
    if (lcm == 0) {
        return;
    }

    Value factor = lcm / _v[k];
    Value rhs_factor = lcm / rhs._v[k];

    Value final_gcd = _v[0];
    for (Index i = 0; i < k; ++i) {
        _v[i] *= factor;
        final_gcd = std::gcd(final_gcd, _v[i]);
    }

    for (Index i = k; i < rhs._v.size(); ++i) {
        _v[i] = _v[i] * factor - rhs._v[i] * rhs_factor;
        final_gcd = std::gcd(final_gcd, _v[i]);
    }
    // Reduce
    if (final_gcd == 1) {
        return;
    }
    for (Index i = 0; i < rhs._v.size(); ++i) {
        _v[i] /= final_gcd;
    }
}

//------------------------------------------------------------------------------

void ParikhVector::AddAttrCnt(Index index, Value cnt)
{
    Resize(index);
    _v[index] += cnt;
}

//------------------------------------------------------------------------------

void ParikhVector::Resize(size_t count)
{
    if (count > _v.size()) {
        _v.resize(count);
    }
}

//------------------------------------------------------------------------------

void ParikhVector::ForceResize(size_t count)
{
    _v.resize(count);
}



//==============================================================================
// class EvLogTSWithParVecs
//==============================================================================


EvLogTSWithParVecs::EvLogTSWithParVecs(IStateIDsPool* stIDsPool)
    : Base(stIDsPool)
{
    _stateParVec[getInitState()] = ParikhVector();
}


//------------------------------------------------------------------------------

EvLogTSWithParVecs::EvLogTSWithParVecs(const EvLogTSWithParVecs& that)
    : BaseEventLogTS(that), _attrInds(that._attrInds)
{
    validateStates(that);
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs& EvLogTSWithParVecs::operator=(const EvLogTSWithParVecs& that)
{
    EvLogTSWithParVecs tmp(that);
    std::swap(tmp, *this);

    return *this;
}

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
            _stateParVec[*stCur] = it->second;

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

    return std::make_pair((*pv).second, true);
}

EvLogTSWithParVecs::ParikhVectorPtr EvLogTSWithParVecs::getParikhVectorPtr(State s)
{
    StateParikhVectorMap::iterator pv = _stateParVec.find(s);
    if (pv == _stateParVec.end())
        return nullptr;
        
    return &((*pv).second);
}

//------------------------------------------------------------------------------

void EvLogTSWithParVecs::setParikhVector(State s, const ParikhVector& pv)
{
    _stateParVec[s] = pv;
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

    Transition tr = Base::getOrAddTrans(s, t, lbl);
    StateParikhVectorMap::iterator pv_it = _stateParVec.find(t);
    if (pv_it == _stateParVec.end()) {
        ParikhVectorRes pvr = getParikhVector(s);
        if (!pvr.second) {
            throw LdopaException("Parikh vector for previous state does not exist");
        }
        pvr.first.AddAttrCnt(index, lblCnt);
        setParikhVector(t, pvr.first);
    } else {
        ParikhVector& pv = (*pv_it).second;
        pv.AddAttrCnt(index, lblCnt);
    }
    return tr;
}

//------------------------------------------------------------------------------

}}} // namespace xi { namespace ldopa { namespace ts {
