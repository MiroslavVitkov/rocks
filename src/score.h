#ifndef SCORE_H_
#define SCORE_H_


// In this file: tools to evaluate the performance of a predictive model.


#include "dat.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/matrix.h>
#endif

#include <functional>
#include <utility>
#include <vector>


namespace score
{


#ifdef CMAKE_USE_DLIB
using Confusion = dlib::matrix< unsigned >;


// Return the spectrum which lowest score.
using Comp = std::function< int( const dat::Spectrum & ) >;
const dat::Spectrum & find_worst( Comp, const dat::Dataset & d );


// Each row corresponds to a class of data and each column to a prediction.
Confusion calc_confusion( const std::vector< label::Num > & ground_truth
                        , const std::vector< label::Num > & predicted );


double accuracy( const Confusion & );
#endif  // CMAKE_USE_DLIB

}  // namespace score

#endif  // #ifndef SCORE_H_
