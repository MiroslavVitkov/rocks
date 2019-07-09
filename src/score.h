#ifndef SCORE_H_
#define SCORE_H_


// In this file: tools to evaluate the performance of a predictive model.


#include "dat.h"

#include <dlib/matrix.h>

#include <functional>
#include <utility>
#include <vector>


namespace score
{


// Return the spectrum which lowest score.
using Comp = std::function< int( const dat::Spectrum & ) >;
const dat::Spectrum & find_worst( Comp, const dat::Dataset & d );


// Each row corresponds to a class of data and each column to a prediction.
dlib::matrix< unsigned > calc_confusion( const std::vector<int> & ground_truth
                                       , const std::vector<int> & predicted );


}  // namespace score

#endif  // #ifndef SCORE_H_
