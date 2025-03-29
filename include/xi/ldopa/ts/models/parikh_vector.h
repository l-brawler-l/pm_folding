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
/// Definitions of classes required for the folding algorithm.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef XI_LDOPA_TRSS_MODELS_PARIKH_VECTOR_H_
#define XI_LDOPA_TRSS_MODELS_PARIKH_VECTOR_H_

#pragma once

// ldopa dll
#include "xi/ldopa/ldopa_dll.h"

// ldopa
#include "xi/ldopa/ts/models/eventlog_ts.h"

// std
#include <algorithm>


namespace xi { namespace ldopa { namespace ts {

/** \brief Declares a Parikh Vector type based on a list of integers.
 */
class LDOPA_API ParikhVector
{
public:
     /** \brief Declares type for integer. */
    typedef int Value;
     /** \brief Declares type for index. */
    typedef size_t Index;
public:
    /** \brief Default constructor. */
    ParikhVector();

    /** \brief Constructor with fixed size \a size of vector. */
    ParikhVector(size_t size);

    /** \brief Default destructor. */
    ~ParikhVector();
public:
    /** \brief Copy constructor. */
    ParikhVector(const ParikhVector& rhs);

    /** \brief Copy (assigning) operator  */
    ParikhVector& operator=(const ParikhVector& rhs);

    /** \brief Swap method for copy operator */
    friend void swap(ParikhVector& lhv,ParikhVector& rhv) 
    {
        // меняем основные поля местами
        std::swap(lhv._v, rhv._v);
    }

public:
    /** \brief \returns an amount of attributes with index \a index saved in parikh vector.
     */
    Value GetAttrCnt(Index index) const;

    /** \brief Adds an amount \a cnt of attributes with index \a index into parikh vector.
     */
    void AddAttrCnt(Index index, Value cnt);

    /** \brief Sets the parikh vector size to greater than or equal to the \a count.
     */
    void Resize(size_t count);

    /** \brief Sets the parikh vector size to the \a count.
     */
    void ForceResize(size_t count);

    /** \brief Subtracts the parikh vector \a rhs so that 
     *  amount of attributes with index \a k is equal to zero.
     *  Then reduces the resulting vector.
     *  Useful function for Gaussian elimination.
     */
    void SubstractSuffix(Index k, const ParikhVector& rhs);

public:
    /** \brief \returns a collection of Attribute counts (by a vector). */
    std::vector<Value>& GetAttrsCnt() { return _v; }

    /** \brief \returns a collection of Attrivute counts (by a vector). / const */
    const std::vector<Value>& GetAttrsCnt() const { return _v; }

public:
    /** \brief \returns a new parikh vector which is the difference between 
     * parikh vectors \a lhs and \a rhs.
     */
    friend ParikhVector GetDiff(ParikhVector lhs, const ParikhVector& rhs);

    /** \brief \returns a result of scalar product between vectors \a lhs and \a rhs.
     */
    friend Value GetScalar(const ParikhVector& lhs, const ParikhVector& rhs);


protected:
    /** \brief A vector of attribute amounts. */
    std::vector<Value> _v;
};


//==============================================================================
//  class Matrix
//==============================================================================

/** \brief Declares a Matrix type based on a list of Parikh vectors.
 */
class LDOPA_API Matrix
{
public:
    /** \brief Declares type for integer. */
    typedef ParikhVector::Value Value;
    /** \brief Declares type for index. */
    typedef ParikhVector::Index Index;
     /** \brief Declares type for vector of indices. */
    typedef std::vector<size_t> IndexVector;
public:
    /** \brief Default constructor. */
    Matrix();

    /** \brief Constructor with fixed size \a limit of each parikh vector. */
    Matrix(size_t limit);

    /** \brief Default destructor. */
    ~Matrix();
public:
    /** \brief Copy constructor. */
    Matrix(const Matrix& rhs);

    /** \brief Copy (assigning) operator  */
    Matrix& operator=(const Matrix& rhs);

public:
    /** \brief Adds a new parikh vector \a pv to matrix. */
    void PushBack(const ParikhVector& pv);

    /** \brief \returns an amount of attributes with index \a j saved in 
     *  parikh vector with index \a i.
     */
    Value GetElem(Index i, Index j);

    /** \brief Performs a Gaussian elimination on matrix.
     *  \returns a vector of indices of free variables in matrix.
    */
    IndexVector GaussianElimination();

    /** \brief \returns a matrix of nullspace of current matrix.
     */
    Matrix GetNullspace();
public:
    /** \brief \returns a collection of Parikh Vectors (by a vector). */
    std::vector<ParikhVector>& getParikhVectors() { return _matrix; }

    /** \brief \returns a collection of Parikh Vectors (by a vector). / const. */
    const std::vector<ParikhVector>& getParikhVectors() const { return _matrix; }
protected:


protected:
    /** \brief A vector of Parikh vectors. */
    std::vector<ParikhVector> _matrix;
    /** \brief A maximum size of all parikh vectors. */
    size_t _maxSize;
    /** \brief A limit for all parikh vectors. */
    size_t _limit;
};





}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_MODELS_PARIKH_VECTOR_H_
