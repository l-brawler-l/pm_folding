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
/// Implementation of freg-based TS reduction algorithm.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_TRSS_ALGOS_TS_CYCLE_CONDENSER_H_
#define XI_LDOPA_TRSS_ALGOS_TS_CYCLE_CONDENSER_H_

//#pragma once


// ldopa dll
#include "xi/ldopa/ldopa_dll.h"

// ldopa
#include "xi/ldopa/ts/models/evlog_ts_fold.h"
#include "xi/ldopa/utils/elapsed_time.h"
#include "xi/ldopa/ts/algos/ts_folding_builder.h"
#include "xi/ldopa/ts/models/eventlog_ts_stateids.h"


// std
#include <set>
#include <queue>

namespace xi { namespace ldopa { namespace ts {   //

/** \brief Determines some additional edge property types and helper method  */
struct EdgeProperty_traits 
{
    struct EdgeProperty
    {
        int index;
    };
    // Functions

    /** \brief Function extracts a trans label from the complex property. */
    static int getIndex(const EdgeProperty& trp) { return trp.index; }

    /** \brief Sets a label of the trans. */
    static void setIndex(EdgeProperty& trp, int index) { trp.index = index; }
}; // class EdgeProperty_traits


class LDOPA_API CycleCondensedTsBuilder;

/** \brief Base class for parikh trie. Primarily defines types.
 *
 *  Parikh trie is internally based on BGL bidigraph class.
 */
class BaseParikhTrie 
{
public:
    /** \brief Type pack for embedding as graph vertices property. */
    typedef typename EdgeProperty_traits::EdgeProperty EdgeProperty;
    
    /** \brief Defining a type for the underlying graph.
     */
    typedef boost::adjacency_list<
        boost::listS,               // out edge storage
        boost::listS,               // vertices storage
        boost::directedS,             // directed      
        // properties
        boost::no_property,               // vertex properties
        EdgeProperty
        //TLabel                    // edge properties
    > Graph;
};
//==============================================================================
//  class ParikhTrie
//==============================================================================


/** \brief Implementation of Parikh Trie based on 
*/
class LDOPA_API ParikhTrie :
public BaseParikhTrie,
protected gr::BoostGraphP<typename BaseParikhTrie::Graph>
{
    friend class CycleCondensedTsBuilder;
public:
    typedef EvLogTSWithParVecs TS;

    typedef ParikhVector::Value Value;
    typedef ParikhVector::Index Index;
public:
    // Transition Properties 
    typedef TS::Activity Activity;
    typedef typename std::vector<const Activity*> ActsPermutation;
public:
    //-----<Types>-----
    typedef BaseParikhTrie::Graph Graph;
    typedef gr::BoostGraphP<Graph> BaseGraph;

    /** \brief State type alias for graph Vertex. */
    typedef typename BaseGraph::Vertex State;          // BoostGraphP

    /** \brief Transition type alias for graph Edge. */
    typedef typename BaseGraph::Edge Transition;

    /** \brief Iterator for transitions modeled by output edges of a graph. */
    typedef typename BaseGraph::OedgeIter OtransIter;

     /** \brief Iterator for transitions modeled by output edges of a graph. */
     typedef typename BaseGraph::EdgeIter TransIter;

    /** \brief A pair of state iterators, which represents a collection of states. */
    typedef typename BaseGraph::VertexIterPair StateIterPair;

    /** \brief A pair of transition iterators, which represents a collection of output edges. */
    typedef typename BaseGraph::OedgeIterPair OtransIterPair;

    /** \brief A pair of transition iterators, which represents a collection of edges. */
    typedef typename BaseGraph::EdgeIterPair TransIterPair;


    /** \brief Type alias for edge/transition returning result. 
     * 
     *  Normally, if the second part of this pair is true, then the result
     *  is presented (found, exists etc), otherwise it is false.
     */
    typedef std::pair<Transition, bool> TransRes;

    /** \brief Type alias for state/vertex returning result with some helper methods. */
    typedef std::pair<State, bool> StateRes;

    typedef std::pair<State, Transition> StateTrans;

    typedef std::map<State, ParikhVector*> StateToPVMap;

    typedef std::map<Value, Transition> Domain;

public:
    /** \brief Constructor. */
    ParikhTrie(TS* ts);

    /** \brief Virtual destructor. */
    virtual ~ParikhTrie();

protected:
    ParikhTrie(const ParikhTrie& other);
    ParikhTrie& operator=(const ParikhTrie&);
public:
    /** \brief Returns a ref to the underlying graph. */
    Graph& getGraph() { return BaseGraph::getGraph(); }

    /** \brief Const overloaded version of ParikhTrie::getGraph(). */
    const Graph& getGraph() const { return BaseGraph::getGraph(); }

public:
    /** \brief Return a number of states in the TS. */
    size_t getStatesNum() const { return BaseGraph::getVerticesNum(); } //{ return _gr.getVerticesNum(); }

    /** \brief Return a number of edges in the TS. */
    size_t getTransitionsNum() const  { return BaseGraph::getEdgesNum(); }//{ return _gr.getEdgesNum(); }

    /** \brief Returns a collection of States (by a pair of state iterators). */
    inline StateIterPair getStates() { return BaseGraph::getVertices(); }

    /** \brief Returns a collection of States (by a pair of state iterators)/Const overload. */
    inline StateIterPair getStates() const { return BaseGraph::getVertices(); }


    /** \brief Returns a collection of Transitions (by a pair of state iterators). */
    inline TransIterPair getTransitions() { return BaseGraph::getEdges(); }

    /** \brief Returns a collection of Transitions (by a pair of state iterators) / const. */
    inline TransIterPair getTransitions() const { return BaseGraph::getEdges(); }

    /** \brief Returns a collection of Output Transitions of a state \a s. */
    inline OtransIterPair getOutTransitions(State s) { return BaseGraph::getOutEdges(s); }

    /** \brief Returns a collection of Output Transitions of a state \a s (const). */
    inline OtransIterPair getOutTransitions(State s) const { return BaseGraph::getOutEdges(s); }

    /** \brief Creates a new state with parikh vector ptr \a pv and returns it. */
    State addState(ParikhVector* pv = nullptr);

    /** \brief Extracts a parikh vector from state.
     */
    ParikhVector* extractParikhVector(State s);

    /** \brief Return bundled property for the given transition \a tr. */
    inline const EdgeProperty& getBundle(const Transition& tr) const { return getGraph()[tr]; }

    /** \brief Extracts a transition label from a complex transition property
     *  through the given transition.
     */
    Value extractTransIndex(const Transition& t) const
    {
        const EdgeProperty& trBundle = getBundle(t);
        return  EdgeProperty_traits::getIndex(trBundle);
    }

    /** \brief Places a label inside labl property whatever it is... */
    static void emplaceTransIndex(EdgeProperty& trp, Value index)
    {
        EdgeProperty_traits::setIndex(trp, index);
    }

    /** \brief Looks over all edges from state \a s
     *  for one labeled with the given label \a lbl. If such a transition exists,
     *  returns true as a second part of a returning value, otherwise returns false there.
     */
    TransRes getTrans(State s, Value lbl);

    /** \brief Gets a target state of transition from state \a s, 
     *  marked by a label \a lbl.
     *
     *  If no transition and target state exist, creates a new ones.
     */
    StateTrans getOrAddTargetState(State s, Value lbl, ParikhVector* pv = nullptr);

    /** \brief Returns the source State (vertex) of the given transitions \param t */
    State getSrcState(const Transition& t) const { return BaseGraph::getSrcVertex(t); }

    /** \brief Returns the target State (vertex) of the given transitions \param t */
    State getTargState(const Transition& t) const { return BaseGraph::getTargVertex(t); }
public:
    /** \brief Builds a parikh trie */
    void build();

    /** \brief Finds all tandem permutations in regions bounded with \a k and stores them in \a pvDiffs */
    void TandemSearch(Matrix& pvDiffs, unsigned int k);
protected:

    /** \brief Proceesses another parikh vector in TS state \a s */
    void processParikhVector(TS::State s);

protected:
    /** \brief Adds into the TS a new transition between the given states \a s and \a t,
     *  labeled by label \a lbl.
     *
     *  Does not check if another transition, which meets the conditions above, exists. 
     *  It should be checked by a caller by convention.
     */
    Transition addTransitionInternal(State s, State t, Value lbl)
    {
        Transition trans = BaseGraph::addEdge(s, t);
        emplaceTransIndex(getGraph()[trans], lbl);

        return trans;
    }

     /** \brief Finds all tandem permutations in regions bounded with \a k 
      * in all leaves of states \a v1 and \a v2 with multiplicity \a i
      * and stores them in \a pvDiffs */
    void Recur(Matrix& pvDiffs, State v1, State v2, unsigned int i);
    
    /** \brief Recur analog but with all leaves in only one state \a v */
    void Recur(Matrix& pvDiffs, State v, unsigned int i);

protected:
    /** \brief Stores a TS based on which parikh trie will be built. */
    TS* _ts;
    /** \brief Stores the initial state of parikh vector. */
    State* _initSt;
    /** \brief Stores a map containing parikh vector ptrs for states in parikh trie. */
    StateToPVMap* _stPV;
    /** \brief Stores a set of all edges from initial state */
    Domain* _initDom;
    /** \brief Stores the maximum number of edges leaving the vertex */
    Value _maxv;
}; // class ParikhTrie



//==============================================================================
//  class CycleCondensedTsBuilder
//==============================================================================


/** \brief Implements an algorithm for building a condensed TS.
*
*  TS that is created during builder work is stored by the builder until next
*  building or the TS is detached by using an appropriate method (detach).
*  Normally, the builder manages the lifetime of a builded TS.
*/
class LDOPA_API CycleCondensedTsBuilder 
        : public ElapsedTimeStore       // вспомогательный класс для учета времени алгоритма
{
public:
    //-----<Types>-----
    /** \brief Alias for the datatype of a currently created TS. */
    typedef EvLogTSWithParVecs TS;

    typedef ParikhVector::Value Value;
    typedef ParikhVector::Index Index;

    /** \brief Set of transitions. */
    typedef std::set<TS::Transition> SetOfTrans;
    typedef SetOfTrans::iterator SetOfTransIter;                ///< Iterator for a set.

public:

    /** \brief Constructor initializes with all necessary data.
     *
     *  \param ts is TS to be condensed.
     * 
     *  \param sf is a state function that will be used with builded TS.
     * 
     *  \param stIDsPool is a pool that builded TS will use.
     */
    CycleCondensedTsBuilder(TS* ts, CondensedStateFunc* sf, FixedIntListStateIdsPool* stIDsPool);


    /** \brief Destructor. */
    ~CycleCondensedTsBuilder();

protected:
    CycleCondensedTsBuilder(const CycleCondensedTsBuilder&);                 // Prevent copy-construction
    CycleCondensedTsBuilder& operator=(const CycleCondensedTsBuilder&);      // Prevent assignment


public:
    /** \brief Builds a condensed TS using a \a k parameter as a boundedness for cycles
     * and \a newStIDsPool as a pool which will be used by the new TS.
     *
     *  \returns a TS if built successfully, otherwise nullptr.
     *  A newly built TS is managed by the builder according to the rules,
     *  presented in the class definition.
     */
    TS* build(unsigned int k); 

    /** \brief Detaches and returns the TS built at the previous call of build() method. */
    TS* detach();

    /** \brief Cleans a previously built TS if exists. If no, does nothing. */
    void cleanTS();

public:
    //----<Setters/Getters>----

    /** \brief Returns a ptr to the the TS built at the previous call of build() method. */
    TS* getTS() { return _ts; }

    /** \brief Const overload for getTS(). */
    const TS* getTS() const { return  _ts; }

    /** \brief Returns a ptr to the source TS. */
    TS* getSrcTS() { return _srcTs; }

    /** \brief Const overload for getSrcTS(). */
    const TS* getSrcTS() const { return _srcTs; }

    /** \brief Returns a boundedness that was used for the very last building. */
    double getBoundedness() const { return _k; }

protected:
    /** \brief Copies all states and transition from _srcTs to _ts 
     * using state function _sf.
    */
    void copyTS();

    /** \brief Copies state \a stOrig from _srcTs to _ts 
     * using state function _sf. \returns copied state.
    */
    TS::State copyState(TS::State stOrig);

protected:

    /** \brief Src TS to be condensed. */
    TS* _srcTs;

    /** \brief Stores a special state function. */
    CondensedStateFunc* _sf;

    /** \brief Stores a ptr to a pool of state IDs. */
    FixedIntListStateIdsPool* _stIDsPool;

    /** \brief Resulting condensed TS. */
    TS* _ts;

    /** \brief Stores a boundedness for cycles. */
    unsigned int _k;

}; // class CycleCondensedTsBuilder


}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_ALGOS_TS_CYCLE_CONDENSER_H_
