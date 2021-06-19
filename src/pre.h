#ifndef PRE_H_
#define PRE_H_


// In this file: preprocessing of a dataset before feeding it to a model.


#include "dat.h"
#include "except.h"

#ifdef CMAKE_USE_SHARK
#include <shark/Algorithms/Trainers/PCA.h>
#endif

#include <vector>


namespace pre
{


struct Base
{
    using Dataset = shark::ClassificationDataset;

    virtual Dataset operator()( const Dataset & ) const = 0;

    virtual ~Base() = default;
};


void logarithm( dat::DataRaw & );
void normalize( dat::DataRaw & );

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
    if( is( "pca" ) )
    {
        return std::make_unique< PCA >( d );
    }

    throw Exception( name + ": no such preprocessing algorithm found. "
                     "See 'pre.h' for a list of all algos."
                   );
}


inline std::unique_ptr< Base > create( const std::string & name
                                     , const dat::Dataset & d )
{
    return create( name, dat::to_shark_dataset( d ) );
}


extern const std::vector< std::string > ALL_PRE;


}  // namespace pre


#endif // defined( PRE_H_ )
