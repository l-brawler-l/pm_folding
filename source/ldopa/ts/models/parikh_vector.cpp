#include "xi/ldopa/ts/models/parikh_vector.h"
#include <numeric> // std::gcd



namespace xi { namespace ldopa { namespace ts {


//==============================================================================
// class ParikhVector
//==============================================================================

ParikhVector::ParikhVector() 
{

}

//------------------------------------------------------------------------------

ParikhVector::ParikhVector(size_t size) 
{
    _v.resize(size);
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
    _v = that._v;

    return *this;
}

//------------------------------------------------------------------------------

ParikhVector::Value ParikhVector::GetAttrCnt(Index index) const
{
    if (index >= _v.size()) {
        return 0;
    }
    return _v[index];
}

//------------------------------------------------------------------------------

void ParikhVector::AddAttrCnt(Index index, Value cnt)
{
    Resize(index + 1);
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

ParikhVector GetDiff(ParikhVector lhs, const ParikhVector& rhs)
{
    lhs.Resize(rhs._v.size());
    for (ParikhVector::Index i = 0; i < rhs._v.size(); ++i) {
        lhs._v[i] -= rhs._v[i];
    }
    return lhs;
}

//------------------------------------------------------------------------------

ParikhVector::Value GetScalar(const ParikhVector& lhs, const ParikhVector& rhs)
{
    ParikhVector::Value ans = 0;
    for (ParikhVector::Index i = 0; i < std::min(lhs._v.size(), rhs._v.size()); ++i) {
        ans += lhs._v[i] * rhs._v[i];
    }
    return ans;
}







//==============================================================================
//  class Matrix
//==============================================================================

Matrix::Matrix()
    : _maxSize(0)
    , _limit(0)
{

}

//------------------------------------------------------------------------------

Matrix::Matrix(size_t limit)
    : _maxSize(0)
    , _limit(limit)
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
    _matrix = rhs._matrix;
    _maxSize = rhs._maxSize;
    _limit = rhs._limit;

    return *this;
}

//------------------------------------------------------------------------------

void Matrix::PushBack(const ParikhVector& pv)
{
    _matrix.push_back(pv);
    _maxSize = std::max(_maxSize, pv.GetAttrsCnt().size());
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
    size_t M = _limit;
    if (M == 0) {
        M = _maxSize;
    }
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
            swap(_matrix[i_max], _matrix[cur_row]);
            //std::iter_swap(_matrix.begin() + i_max, _matrix.begin() + cur_row);
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

    Matrix nullspace;

    if (_limit == 0) {
        pivotVars.reserve(_maxSize - freeVars.size());
        nullspace = Matrix(_maxSize);
    } else {
        pivotVars.reserve(_limit - freeVars.size());
        nullspace = Matrix(_limit);
    }
    

    size_t fv_ind = 0; // индекс по вектору свободных переменных

    
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

            if (free == 0) {
                continue;
            }

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

            Value coef = -(free * factor) / pivot;
            solution.AddAttrCnt(pivotVars[j], coef);
        }
        // добавляем в ответ
        nullspace.PushBack(solution);
        // смотрим на следующий индекс свободной переменной
        ++fv_ind;

    }
    return nullspace;
}



}}} // namespace xi { namespace ldopa { namespace ts {
