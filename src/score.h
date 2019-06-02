#ifndef SCORE_H_
#define SCORE_H_


// In this file: tools to evaluate the performance of a predictive model.


#include "io.h"

#include <utility>


namespace score
{


std::pair< io::Dataset, io::Dataset >
train_test_split( const io::Dataset &
                , double test_proportion = 0.2 );

void evaluate_and_print( const std::vector<int> & targets
                       , const std::vector<int> & outputs );


}  // namespace score

#endif  // #ifndef SCORE_H_
