#ifndef SCORE_H_
#define SCORE_H_


#include "io.h"

#include <tuple>


namespace score
{


std::tuple< io::Dataset, io::Dataset >
train_test_split( const io::Dataset &
                , double test = 0.2 );

void evaluate_and_print( std::vector<io::Label> targets
                       , std::vector<io::Label> outputs );


}  // namespace score

#endif  // #ifndef SCORE_H_
