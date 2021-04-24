#ifndef DIM_H_
#define DIM_H_


// In this file: dimensionality reduction techniques.


#include "dat.h"

#ifdef CMAKE_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#include <array>
#include <vector>


namespace dim
{


#ifdef CMAKE_USE_OPENCV


// Linear Discriminant Analysis
// Z*x-M maps x into a space where x vectors that share the same class label
// are near each other.
// LDA (at least the implementation in sklearn) can produce at most
// k-1 components (where k is number of classes).
struct LDA
{
    using T = dat::Compressed::value_type;

    LDA( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const;

private:
    mutable cv::LDA _lda;
};


struct PCA
{
    using T = dat::Compressed::value_type;

    PCA( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const;

private:
    cv::PCA _pca;
};


#endif  // CMAKE_USE_OPENCV


// An agegate of simple measures.
// Intended as a placeholder when no libs are enabled.
struct Simple
{
    using T = dat::Compressed::value_type;

    Simple( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const;
};



}  // namespace dim


#endif  // #ifndef DIM_H_
