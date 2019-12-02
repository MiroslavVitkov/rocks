#ifndef DIM_H_
#define DIM_H_


// In this file: dimensionality reduction techniques.


#include "dat.h"

#include <dlib/matrix.h>

#include <array>
#include <vector>


namespace dim
{


// Linear Discriminant Analysis
// Z*x-M maps x into a space where x vectors that share the same class label
// are near each other.
struct LDA
{
    using T = dat::Compressed::value_type;

    LDA( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const;

private:
    dlib::matrix< T > _Z;
    dlib::matrix< T, 0, 1 > _M;
};


// Principal Component Analysis
// Based on the paper:
// 'A New Discriminant Principal Component Analysis Method with
// Partial Supervision (2009) by Dan Sun and Daoqiang Zhang'
struct PCA
{
    using T = dat::Compressed::value_type;

    PCA( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const;

private:
    dlib::matrix< T > _Z;
    dlib::matrix< T, 0, 1 > _M;
};


}  // namespace dim


#endif  // #ifndef DIM_H_
