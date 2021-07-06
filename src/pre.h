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


#ifdef CMAKE_USE_SHARK
struct PCA : Base
{
    PCA( const dat::Dataset & train, unsigned dim=100 );
    PCA( const shark::ClassificationDataset & train, unsigned dim=100 );
    dat::Dataset operator()( const dat::Dataset & ) const override;

    const shark::LinearModel<> _enc;
};
#endif  // CMAKE_USE_SHARK


inline std::unique_ptr< Base > create( const std::string & name
                                     , const dat::Dataset & d
                                     )
{
    const auto is = [ & name ] ( const char * p )
    {
        return ( name.compare( p ) == 0 );
    };

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


extern const std::vector< std::string > ALL_PRE;


}  // namespace pre


#endif // defined( PRE_H_ )
