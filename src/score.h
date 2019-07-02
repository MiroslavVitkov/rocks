#ifndef SCORE_H_
#define SCORE_H_


// In this file: tools to evaluate the performance of a predictive model.


#include "io.h"

#include <functional>
#include <utility>


namespace score
{


void evaluate_and_print( const std::vector<int> & targets
                       , const std::vector<int> & outputs );

// Return the spectrum which lowest score.
using Comp = std::function< int( const io::Spectrum & ) >;
const io::Spectrum & find_worst( Comp, const io::Dataset & d );


}  // namespace score

#endif  // #ifndef SCORE_H_
