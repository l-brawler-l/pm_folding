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


class LDOPA_API ParikhVector
{
public:
    typedef int Value;
    typedef size_t Index;
public:
    ParikhVector(); /// TODO: С помощью функции лога GetActivitiesNum сразу обозначать размер массива

    ~ParikhVector();
public:
    ParikhVector(const ParikhVector& rhs);

    ParikhVector& operator=(const ParikhVector& rhs);

    /** \brief Swap method for copy operator. */
    friend void swap(ParikhVector& lhv,ParikhVector& rhv) 
    {
        // меняем основные поля местами!
        std::swap(lhv._v, rhv._v);
    }

public:
    Value GetAttrCnt(Index index) const;

    void AddAttrCnt(Index index, Value cnt);

    void Resize(size_t count);

    void ForceResize(size_t count);

    void SubstractSuffix(Index k, const ParikhVector& rhs);

public:
    /** \brief Returns a collection of Attribute counts (by a vector). */
    std::vector<Value>& GetAttrsCnt() { return _v; }

    /** \brief Returns a collection of Attrivute counts (by a vector). / const */
    const std::vector<Value>& GetAttrsCnt() const { return _v; }

public:
    friend ParikhVector GetDiff(ParikhVector lhs, const ParikhVector& rhs);

    friend Value GetScalar(const ParikhVector& lhs, const ParikhVector& rhs);


protected:
    std::vector<Value> _v;
};


//==============================================================================
//  class Matrix
//==============================================================================

class LDOPA_API Matrix
{
public:
    typedef ParikhVector::Value Value;
    typedef ParikhVector::Index Index;

    typedef std::vector<size_t> IndexVector;
public:
    Matrix();

    Matrix(size_t limit);

    ~Matrix();
public:
    Matrix(const Matrix& rhs);

    Matrix& operator=(const Matrix& rhs);

public:
    void PushBack(const ParikhVector& pv);

    Value GetElem(Index i, Index j);

    IndexVector GaussianElimination();

    Matrix GetNullspace();
public:
    /** \brief Returns a collection of Parikh Vectors (by a vector). */
    std::vector<ParikhVector>& getParikhVectors() { return _matrix; }

    /** \brief Returns a collection of Parikh Vectors (by a vector). / const. */
    const std::vector<ParikhVector>& getParikhVectors() const { return _matrix; }
protected:


protected:
    std::vector<ParikhVector> _matrix;
    size_t _maxSize;
    size_t _limit;
};





}}} // namespace xi { namespace ldopa { namespace ts {


#endif // XI_LDOPA_TRSS_MODELS_PARIKH_VECTOR_H_
