#ifndef PRE_H_
#define PRE_H_


// In this file: preprocessing of a dataset before feeding it to a model.


#include "dat.h"


namespace pre
{


void logarithm( dat::DataRaw & );
void normalize( dat::DataRaw & );


}  // namespace pre


#endif // defined( PRE_H_ )
