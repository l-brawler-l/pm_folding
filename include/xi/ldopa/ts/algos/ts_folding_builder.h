////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief     LDOPA Transision Systems Library
/// \author    Sergey Shershakov
/// \version   0.2.0
/// \date      11.10.2017
/// \copyright (c) xidv.ru 2014–2017.
///            This source is for internal use only — Restricted Distribution.
///            All rights reserved.
///
/// Tools for building transition systems from a log by using simple state functions.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_TRSS_ALGOS_TS_FOLDING_BUILDER_H_
#define XI_LDOPA_TRSS_ALGOS_TS_FOLDING_BUILDER_H_

//#pragma once


// ldopa dll
#include "xi/ldopa/ldopa_dll.h"

// ldopa
#include "xi/ldopa/ts/models/evlog_ts_fold.h"
#include "xi/ldopa/eventlog/eventlog.h"
#include "xi/ldopa/ts/models/eventlog_ts_stateids.h"
#include "xi/ldopa/ts/algos/ts_simple_builder.h"
#include "xi/ldopa/utils/progress_callback.h"
#include "xi/ldopa/utils/elapsed_time.h"


namespace xi { namespace ldopa { namespace ts {
//==============================================================================
//  class TsFoldBuilder
//==============================================================================


/** \brief Implements an algorithm for building a TS with a custom state function.
 *
 *  TS that is created during builder work is stored by the builder until next
 *  building or the TS is detached by using an appropriate method (detach).
 *  Normally, the builder manages the lifetime of a builded TS.
 */
class LDOPA_API TsFoldBuilder 
        : public ElapsedTimeStore       // вспомогательный класс для учета времени алгоритма
{
public:
    //-----<Types>-----

    /** \brief Enumerates setting bit number constants. */
    enum
    {
        SET_AUTO_CLEAR_ST_POOL = 0,     ///< Determines whether the stateIDs pool is cleared before mining.
        //SET_SINGLE_ACPT_STATE = 1,      ///< Determines whether the single accepting state must be added.
        //
        //
        SET__LAST                       ///< Technical internal value used to determine size for underlying bitset
    };

    /** \brief A typename for bitset type for storing settings */
    typedef std::bitset<SET__LAST> SettingsBitset;

    /** \brief Alias for the datatype of a currently created TS. */
    typedef EvLogTSWithParVecs TS;
    typedef TS::State TsState;          ///< TS state.
    typedef TS::StateIter TsStateIter;  ///< TS state iterator
public:
    // types
    typedef eventlog::IEventLog IEventLog;        ///< Alias for IEventLog2.
    typedef eventlog::IEventTrace IEventTrace;    ///< Alias for IEventTrace2.

    /** \brief Declares a set of log attributes. */
    typedef std::set<IEventLog::Attribute> AttributesSet;

    typedef IEventLog::Attribute Attribute;     // Added as of 18/08/2018: надо везд заменить упрощенкой

public:
    //-----<Const>-----

    // default settings
    static const unsigned int DEF_SETTINGS =
        (0x0 << SET_AUTO_CLEAR_ST_POOL          // Do not clear stateIDs map by default.
        //| 0x0 << SET_SINGLE_ACPT_STATE          // With no single accepting state so far.
        //| 0x1 <<                              //
        );
public:
    //----<Constructors and destructor>----
    
    /** \brief Constructor initializes the builder with a log and a state function. */
    TsFoldBuilder(IEventLog* log, ITsStateFunc* sf, IStateIDsPool* stIDsPool);

    /** \brief Destructor. */
    ~TsFoldBuilder();

protected:
    TsFoldBuilder(const TsFoldBuilder&);                 // Prevent copy-construction
    TsFoldBuilder& operator=(const TsFoldBuilder&);      // Prevent assignment

public:
    //----<Main interface>----
    
    /** \brief Builds a TS. 
     *
     *  \returns a TS if built successfully, otherwise nullptr.
     *  A newly built TS is managed by the builder according to the rules,
     *  presented in the class definition.
     */
    TS* build();

    /** \brief Detaches and returns the TS built at the previous call of build() method. */    
    TS* detach();

    /** \brief Cleans a previously built TS if exists. If no, does nothing. */
    void cleanTS();

public:
    //----<Setters/Getters>----

    /** \brief Returns a ptr to the the TS built at the previous call of build() method. */
    TS* getTS() { return _ts; }

    /** \brief Const overload for getTS(). */
    const TS* getTS() const { return _ts; }

    /** \brief Sets a progress callback. */
    void setProgressCB(IProgressCallback* pcb) { _progrCB = pcb; }

    /** \brief Returns a progress callback. */
    IProgressCallback* getProgressCB() const { return _progrCB; }
public:
    //-----<Helpers for work with settings>-----

    /** \brief Getter for SET_AUTO_CLEAR_ST_POOL option. */
    bool isAutoClearStateIDsPool() const { return _settings[SET_AUTO_CLEAR_ST_POOL]; }

    /** \brief Setter for SET_AUTO_CLEAR_ST_POOL option. */
    void setAutoClearStateIDsPool(bool f) { _settings[SET_AUTO_CLEAR_ST_POOL] = f; }

protected:
    //----<Algorith decomposition>----
    
    /** \brief Proceesses anothoer trace from the log. */
    void processTrace(int i);

    // альтернативный подход с трассой, как объектом
    void processTrace(IEventTrace* tr);

    /** \brief Makes a new (single) accepting state and flows all previous AS to it. */
    void makeSingleAcptState();
protected:
    /** \brief Settings (flags) object. */
    SettingsBitset _settings;

    /** \brief Stores a ptr to an event log.  */
    IEventLog* _log;

    /** \brief Stores a state function. */
    ITsStateFunc* _sf;

    /** \brief Stores a ptr to a pool of state IDs. */
    // #thinkof: здесь у нас идет тот же пул, что и в функции... если они разные?!
    // должна ли функция стать более сложным компонентом — создающим дефолтную TS в т.ч.?
    // например, ITsElementsCreator?
    IStateIDsPool* _stIDsPool;

    /** \brief Stores a resulting TS. */
    TS* _ts;

    /** \brief Stores the number of all traces in the log, processed at the last build. */
    int _tracesNum;

    /** \brief Progress callback. */
    IProgressCallback* _progrCB;

    /** \brief Store last progress value to decrease the frequency of progress updater. */
    int _lastProgress;

}; //  class TsFoldBuilder

}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_ALGOS_TS_SIMPLE_BUILDER_H_
