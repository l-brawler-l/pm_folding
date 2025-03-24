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
#include "xi/ldopa/ts/models/parikh_vector.h"


namespace xi { namespace ldopa { namespace ts {
/** \brief Definition for Event Log TS containing additional attributes.
 *
 *  Additional attributes: Parikh vectors for each state.
 */
class LDOPA_API EvLogTSWithParVecs : public BaseEventLogTS
{
public:
    //-----<Types>-----

    /** \brief Enumerates setting bit number constants. */
    enum
    {
        SET_IGNORE_DIFFS = 0,     ///< Determines whether parikh vector differences are ignored.
        SET__LAST                       ///< Technical internal value used to determine size for underlying bitset
    };

    /** \brief A typename for bitset type for storing settings */
    typedef std::bitset<SET__LAST> SettingsBitset;

    
    typedef BaseEventLogTS Base;

    typedef ParikhVector::Value Value;

    typedef ParikhVector::Index Index;

    /** \brief Mapping of states to parikh vectors. */
    typedef std::map<State, ParikhVector> StateParikhVectorMap;

    /** \brief pointer to Parikh Vector. */
    typedef ParikhVector* ParikhVectorPtr;

    /** \brief Datatype for an integer resulting value that can be undefined. */
    typedef std::pair<ParikhVector, bool> ParikhVectorRes;
  
    /** \brief Type for set of attributes. */
    typedef std::map<Attribute, Index> AttrIndexMap;

    /** \brief Datatype for an integer resulting value that can be undefined. */
    typedef std::pair<Index, bool> IndexRes;

public:
    //-----<Const>-----

    // default settings
    static const unsigned int DEF_SETTINGS =
        (0x1 << SET_IGNORE_DIFFS          // Ignore differences by default.
        );

public:
    /** \brief Constructor initializes the TS by a pool of events.
     *
     *  \param stIDsPool is a (possibly shared) pool of state IDs.
     */
    EvLogTSWithParVecs(IStateIDsPool* stIDsPool);

    /** \brief Constructor initializes the TS by a pool of events.
     *
     *  \param stIDsPool is a (possibly shared) pool of state IDs.
     * 
     *  \param attrInds is a map that connects attributes to indexes in parikh vectors.
     *  Useful, if you want to set the order of attributes to determine parikh vector's form.
     * 
     */
    EvLogTSWithParVecs(IStateIDsPool* stIDsPool, const AttrIndexMap& attrInds);

    /** \brief Copy constructor. 
     *
     *  It is crucial to make a right copy for _stateParVec.
     */     
    EvLogTSWithParVecs(const EvLogTSWithParVecs& that);

    /** \brief Copy constructor. */
    // EvLogTSWithParVecs& operator=(const EvLogTSWithParVecs& that);

    /** \brief Virtual Destructor. */
    virtual ~EvLogTSWithParVecs();

public:
    //----<Updated interface>----
    /** \brief Gets a transition between given pair of states \a s and \a t,
     *  marked by a label \a lbl.
     *
     *  If no transition exists, creates a new one.
     */
    Transition getOrAddTrans(State s, State t, const Attribute& lbl);
    //----<Extended interface>----

    /** \brief Gets a transition between given pair of states \a s and \a t,
     *  marked by a label \a lbl, taking into account parikh vectors. 
     *
     *  If the same transition appears again and again, parikh vector of state \a t is increased
     *  by adding attribute \a lbl \a lblCnt times.
     *  If no transition exists, creates a new one.
     *
     *  If you do not want to change parikh vector of state \a t, use getOrAddTrans insted.
     */
    Transition getOrAddTransPV(State s, State t, const Attribute& lbl, Value lblCnt);

    // /** \brief Creates parikh vector for state \a cur by copying parikh vector from
    //  * state \a prev and adding to it attribute \a lbl \a lblCnt times.
    //  */
    // void createParikhVector(State prev, State cur, const Attribute& lbl, Value lblCnt);

public:
    //----<Working with attributes>----

    /** \brief Returns a parikh vector for the given transition \a t. 
     *
     *  If a parikh vector has been set, the second parameter of the resulting object
     *  is true, otherwise false.
     */
    ParikhVectorRes getParikhVector(State s) const;

    /** \brief Returns a parikh vector pointer for the given transition \a t. 
     *
     *  If a parikh vector has been set, returns nullptr.
     */
    ParikhVectorPtr getParikhVectorPtr(State s);

    /** \brief Sets a new value \a pv of parikh vector ptr for the given state \a s. */
    void setParikhVector(State s, const ParikhVector* pv);

    /** \brief Returns count of different attributes in TS. */
    size_t getAttributeNum() const { return _attrInds.size(); }

    /** \brief Returns a map that connects attributes to indexes in parikh vectors. */
    const AttrIndexMap& getMapOfAttrsToIndexes() const { return _attrInds; }

    /** \brief Returns how many times attribute \a lbl has been encountered on 
     * the way from the initial state to state \a s.
     */
    Value getStateAttrCnt(State s, const Attribute& lbl) const;

    // /**  \brief Adds ttribute \a lbl \a lblCnt times to Parikh Vector of state \a s.
    //  */
    // void addActToParikhVector(State s, const Attribute& act, Value lblCnt);

public:
    //-----<Helpers for work with settings>-----

    /** \brief Getter for SET_IGNORE_DIFFS option. */
    bool isIgnoreDiffs() const { return _settings[SET_IGNORE_DIFFS]; }

    /** \brief Setter for SET_IGNORE_DIFFS option. */
    void setIgnoreDiffs(bool f) { _settings[SET_IGNORE_DIFFS] = f; }

protected:
    //----<Construction helpers>----
    
    /** \brief Makes all state-corresponding structures valid after copying the underlying graph. 
     *
     *  \param that is a TS, from which a copy (into this TS) has been done.
     */
    void validateStates(const EvLogTSWithParVecs& that);

    /** \brief Returns index of attribute \a lbl in parikh vectors.
     *  If attribute \a lbl is not found, returns -1.
     */
    IndexRes GetAttrIndex(const Attribute& lbl) const;

    /** \brief Returns index of attribute \a lbl in parikh vectors.
     *  If attribute \a lbl is not found, creates a new index for it.
     */
    Index GetOrAddAttrIndex(const Attribute& lbl);

protected:
    /** \brief Settings (flags) object. */
    SettingsBitset _settings;

    /** \brief Stores a map of states to parikh vector. */
    StateParikhVectorMap _stateParVec;

    /** \brief Stores a map that connects attributes to indexes in parikh vectors. */
    AttrIndexMap _attrInds;

    /** \brief Stores parikh vectors differences rhat equivalent to zero. */
    Matrix _diffs;

}; // class EvLogTSWithFreqs


}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_MODELS_EVLOG_TS_RED_H_
