#ifndef PRE_H_
#define PRE_H_


// In this file: preprocessing of a dataset before feeding it to a model.


#include "dat.h"

#include <vector>


namespace pre
{


void logarithm( dat::DataRaw & );
void normalize( dat::DataRaw & );

// 'ret[ 0 ]' is the index of most important frequency.
// Next is 'ret[ 1 ]' etc.
std::vector< size_t > rank_features( const dat::Dataset & );


}  // namespace pre


#endif // defined( PRE_H_ )
