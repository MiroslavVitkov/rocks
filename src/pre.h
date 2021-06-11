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
    virtual shark::ClassificationDataset operator()( const dat::Dataset & ) const = 0;
    //shark::ClassificationDataset operator()( const dat::Dataset & ) const;

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
    PCA( const dat::Dataset & train, unsigned dim=100 );
    PCA( const shark::ClassificationDataset & train, unsigned dim=100 );

    shark::RealVector encode( const dat::Spectrum & ) const;
    shark::ClassificationDataset encode( const dat::Dataset & ) const;

    //void apply( dat::Dataset & ) const override;
    shark::ClassificationDataset operator()( const dat::Dataset & ) const override;

    const shark::LinearModel<> _enc;
};
shark::RealVector to_shark_vector( const dat::Spectrum & s );
#endif  // CMAKE_USE_SHARK


}  // namespace pre


#endif // defined( PRE_H_ )
