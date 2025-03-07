#include "xi/ldopa/ts/models/evlog_ts_fold.h"



namespace xi { namespace ldopa { namespace ts {


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
    : BaseEventLogTS(that)
{
    validateStates(that);
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

EvLogTSWithParVecs::ParikhVectorRes EvLogTSWithParVecs::getParikhVector(State s) const
{
    StateParikhVectorMap::const_iterator pv = _stateParVec.find(s);
    if (pv == _stateParVec.end())
        return std::make_pair(ParikhVector(), false);   

    return std::make_pair((*pv).second, true);
}

//------------------------------------------------------------------------------

void EvLogTSWithParVecs::setParikhVector(State s, const ParikhVector& pv)
{
    _stateParVec[s] = pv;
}

//------------------------------------------------------------------------------

EvLogTSWithParVecs::Transition EvLogTSWithParVecs::getOrAddTransPV(State s, State t, 
    const Attribute& lbl, int lblCnt) 
{
    Transition tr = Base::getOrAddTrans(s, t, lbl);
    StateParikhVectorMap::iterator pv_it = _stateParVec.find(t);
    if (pv_it == _stateParVec.end()) {
        ParikhVectorRes pvr = getParikhVector(s);
        if (!pvr.second) {
            throw LdopaException("Parikh vector for previous state does not exist");
        }
        ParikhVector::iterator pv_lbl = pvr.first.find(lbl);
        if (pv_lbl == pvr.first.end()) {
            pvr.first[lbl] = lblCnt;
        } else {
            (*pv_lbl).second += lblCnt;
        }
        setParikhVector(t, pvr.first);
    } else {
        ParikhVector& pv = (*pv_it).second;
        ParikhVector::iterator pv_lbl = pv.find(lbl);
        if (pv_lbl == pv.end()) {
            pv[lbl] = lblCnt;
        } else {
            (*pv_lbl).second += lblCnt;
        }
    }
    return tr;
}

//------------------------------------------------------------------------------

}}} // namespace xi { namespace ldopa { namespace ts {
