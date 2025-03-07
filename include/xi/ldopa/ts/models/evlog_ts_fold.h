////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     LDOPA Transision Systems Library
/// \author    Ramil Khamzin
/// \version   0.1.0
/// \date      07.03.2025
/// \copyright (c) xidv.ru 2014–2017.
///            This source is for internal use only — Restricted Distribution.
///            All rights reserved.
///
/// Definitions of EventLog TS that are suitable for folding.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_TRSS_MODELS_EVLOG_TS_FOLD_H_
#define XI_LDOPA_TRSS_MODELS_EVLOG_TS_FOLD_H_

#pragma once

// ldopa dll
#include "xi/ldopa/ldopa_dll.h"

// ldopa
#include "xi/ldopa/ts/models/eventlog_ts.h"


namespace xi { namespace ldopa { namespace ts {



/** \brief Definition for Event Log TS containing additional attributes.
 *
 *  Additional attributes: Parikh vectors for each state.
 */
class LDOPA_API EvLogTSWithParVecs : public BaseEventLogTS
{
public:
#pragma region Type Definitions
    typedef BaseEventLogTS Base;

    typedef std::map<Activity, int> ParikhVector;



    /** \brief Mapping of states to parikh vectors. */
    typedef std::map<State, ParikhVector> StateParikhVectorMap;

    /** \brief Datatype for an integer resulting value (ref) that can be undefined. */
    typedef std::pair<ParikhVector, bool> ParikhVectorRes;

#pragma endregion // Type Definitions

public:
    /** \brief Constructor initializes the TS by a pool of events.
     *
     *  \param stIDsPool is a (possibly shared) pool of state IDs.
     */
    EvLogTSWithParVecs(IStateIDsPool* stIDsPool); // , bool copyPool = false);


    /** \brief Copy constructor. 
     *
     *  It is crucial to make a right copy for _stateParVec.
     */     
    EvLogTSWithParVecs(const EvLogTSWithParVecs& that);

    /** \brief Virtual Destructor. */
    virtual ~EvLogTSWithParVecs();

public:
    //----<Extended interface>----

    /** \brief Gets a transition between given pair of states \a s and \a t,
     *  marked by a label \a lbl, taking into account parikh vectors. 
     *
     *  If the same transition appears again and again, parikh vector of state \a t is increased
     *  by adding activity \a lbl \a lblCnt times.
     *  If no transition exists, creates a new one.
     *
     *  If you do not want to change parikh vector of state \a t, use Base::getOrAddTrans insted.
     */
    Transition getOrAddTransPV(State s, State t, const Attribute& lbl, int lblCnt);

    // /** \brief Creates parikh vector for state \a cur by copying parikh vector from
    //  * state \a prev and adding to it activity \a lbl \a lblCnt times.
    //  */
    // void createParikhVector(State prev, State cur, const Attribute& lbl, int lblCnt);

public:
    //----<Working with attributes>----

    /** \brief Returns a parikh vector for the given transition \a t. 
     *
     *  If a parikh vector has been set, the second parameter of the resulting object
     *  is true, otherwise false.
     */
    ParikhVectorRes getParikhVector(State s) const;

    /** \brief Sets a new value \a pv of parikh vector for the given state \a s. */
    void setParikhVector(State s, const ParikhVector& pv);

    // /**  \brief Adds activity \a lbl \a lblCnt times to Parikh Vector of state \a s.
    //  */
    // void addActToParikhVector(State s, const Attribute& act, int lblCnt);

protected:
    //----<Construction helpers>----
    
    /** \brief Makes all state-corresponding structures valid after copying the underlying graph. 
     *
     *  \param that is a TS, from which a copy (into this TS) has been done.
     */
    void validateStates(const EvLogTSWithParVecs& that);

protected:
    /** \brief Stores a map of states to parikh vector. */
    StateParikhVectorMap _stateParVec;

}; // class EvLogTSWithFreqs


}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_MODELS_EVLOG_TS_RED_H_
