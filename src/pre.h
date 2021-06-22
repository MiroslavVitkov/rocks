#ifndef PRE_H_
#define PRE_H_


// In this file: preprocessing of the dataset before feeding it to a model:
//    - map intensity values to another domain
//    - dimensionalty reduction


#include "dat.h"
#include "except.h"

#ifdef CMAKE_USE_SHARK
#include <shark/Algorithms/Trainers/PCA.h>
#endif

#ifdef CMAKE_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#include <vector>


namespace pre
{


struct Base
{
    virtual dat::Dataset operator()( const dat::Dataset & ) const = 0;

    virtual ~Base() = default;
};


// Apply log10 to each intensity.
struct Log : Base
{
    Log( const dat::Dataset & );
    dat::Dataset operator()( const dat::Dataset & ) const override;
};


// Make all intensities have mean == 0 and variance == 1.
// Probably incorrectly implemented.
struct Norm : Base
{
    Norm( const dat::Dataset & );
    dat::Dataset operator()( const dat::Dataset & ) const override;
};

// 'ret[ 0 ]' is the index of most important frequency.
// Next is 'ret[ 1 ]' etc.
std::vector< size_t > rank_features( const dat::Dataset & );

#ifdef CMAKE_USE_DLIB
dat::Dataset lda( const dat::Dataset & d );
#endif

#ifdef CMAKE_USE_SHARK
struct PCA : Base
{
    using Dataset69 = shark::ClassificationDataset;

    PCA( const dat::Dataset & train, unsigned dim=100 );
    PCA( const shark::ClassificationDataset & train, unsigned dim=100 );

    shark::RealVector encode( const dat::Spectrum & ) const;
    Dataset69 encode( const dat::Dataset & ) const;
    Dataset69 encode( const Dataset69 & ) const;

    //void apply( dat::Dataset & ) const override;
    //shark::ClassificationDataset operator()( const dat::Dataset & ) const;
    dat::Dataset operator()( const dat::Dataset & ) const override;

    const shark::LinearModel<> _enc;
};
#endif  // CMAKE_USE_SHARK


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


struct cvPCA
{
    using T = dat::Compressed::value_type;

    cvPCA( const dat::Dataset & );
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


inline std::unique_ptr< Base > create( const std::string & name
                                     , const shark::ClassificationDataset & d
                                     )
{
    const auto is = [ & name ] ( const char * p )
        { return ( name.compare( p ) == 0 ); };

    if( is( "log" ) )
    {
        return std::make_unique< Log >( d );
    }
    if( is( "norm" ) )
    {
        return std::make_unique< Norm >( d );
    }
    if( is( "pca" ) )
    {
        return std::make_unique< PCA >( d );
    }

    throw Exception( name + ": no such preprocessing algorithm found. "
                     "See 'pre.h' for a list of all algos."
                   );
}


inline std::unique_ptr< Base > create( const std::string & name
                                     , const dat::DataRaw && d
                                     , const label::Codec & c
                                     )
{
    auto encoded{ dat::encode( d, c ) };
    return create( name, encoded );
}


inline std::unique_ptr< Base > create( const std::string & name
                                     , const shark::ClassificationDataset & d
                                     , const label::Codec & c
                                     )
{
    const auto converted{ dat::from_shark_dataset( d ) };
    auto encoded{ dat::encode( converted, c ) };
    return create( name, encoded );
}


extern const std::vector< std::string > ALL_PRE;


}  // namespace pre


#endif // defined( PRE_H_ )
