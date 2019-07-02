#ifndef SCORE_H_
#define SCORE_H_


// In this file: tools to evaluate the performance of a predictive model.


#include "dat.h"

#include <functional>
#include <utility>


namespace score
{


void evaluate_and_print( const std::vector<int> & targets
                       , const std::vector<int> & outputs );

// Return the spectrum which lowest score.
using Comp = std::function< int( const dat::Spectrum & ) >;
const dat::Spectrum & find_worst( Comp, const dat::Dataset & d );


}  // namespace score

#endif  // #ifndef SCORE_H_
