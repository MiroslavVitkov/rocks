#ifndef DIM_H_
#define DIM_H_


// In this file: dimensionality reduction techniques.


#include "dat.h"
#include "except.h"

#ifdef CMAKE_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#include <array>
#include <vector>


namespace dim
{


struct Base
{
    virtual dat::Compressed operator()( const dat::Spectrum & ) const = 0;
    dat::DatasetCompressed operator()( const dat::Dataset & ) const;
    virtual ~Base() = default;
};


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
struct Simple : Base
{
    Simple( const dat::Dataset & );
    dat::Compressed operator()( const dat::Spectrum & ) const override;
};



inline std::unique_ptr< Base > create( const std::string & name
                                     , const dat::Dataset & d )
{
    const auto is = [ & name ] ( const char * p )
        { return ( name.compare( p ) == 0 ); };

    if( is( "simple" ) )
    {
        return std::make_unique< Simple >( d );
    }

    throw Exception( name + ": no such reduction algo found. "
                     "See 'dim.h' for a list of all algos." );
}


extern const std::vector< std::string > ALL;




}  // namespace dim


#endif  // #ifndef DIM_H_
