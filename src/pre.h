#ifndef PRE_H_
#define PRE_H_


// In this file: preprocessing of a dataset before feeding it to a model.


#include "dat.h"

#ifdef CMAKE_USE_SHARK
#include <shark/Algorithms/Trainers/PCA.h>
#endif

#include <vector>


namespace pre
{


void logarithm( dat::DataRaw & );
void normalize( dat::DataRaw & );

// 'ret[ 0 ]' is the index of most important frequency.
// Next is 'ret[ 1 ]' etc.
std::vector< size_t > rank_features( const dat::Dataset & );

#ifdef CMAKE_USE_DLIB
dat::Dataset lda( const dat::Dataset & d );
#endif

#ifdef CMAKE_USE_SHARK
struct PCA
{
    PCA( const dat::Dataset & train, unsigned dim=100 );
    shark::RealVector encode( const dat::Spectrum & ) const;
    shark::LabeledData< shark::RealVector, label::Num >
        encode( const dat::Dataset & ) const;

    const shark::LinearModel<> _enc;
};
shark::RealVector to_shark_vector( const dat::Spectrum & s );
#endif  // CMAKE_USE_SHARK


}  // namespace pre


#endif // defined( PRE_H_ )
