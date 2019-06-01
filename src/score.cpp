#include "score.h"

#include <evaluation.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <set>


namespace score
{


std::tuple< io::Dataset, io::Dataset >
train_test_split( const io::Dataset & d
                , double test_proportion )
{
    io::Dataset train, test;
    for( const auto & kv : d )
    {
        // Stratified sampling.
        // For each label, draw randomly $test proportion of samples.
        std::set<unsigned> indices {};
        std::default_random_engine generator;
        std::uniform_int_distribution<unsigned>
                distribution( 0, static_cast<unsigned>( kv.second.size() ) );
        while( indices.size() < kv.second.size() * test_proportion )
        {
            const auto rand = distribution( generator );
            indices.insert( rand );
        }

        // Copy each datapoint exactly once.
        for( unsigned i = 0; i < kv.second.size(); ++i )
        {
            if( indices.count( i ) )
            {
                test[ kv.first ].push_back( kv.second[ i ] );
            }
            else
            {
                train[ kv.first ].push_back( kv.second[ i ] );
            }
        }
        assert( train[ kv.first ].size()
              + test[ kv.first ].size()
                == d.at( kv.first ).size() );
    }

    return { train, test };
}


void evaluate_and_print( std::vector<io::Label> targets
                       , std::vector<io::Label> outputs)
{
    // Expected format by the evaluation library.
    std::vector<std::vector<double>> targ, outp;

    auto it = targets.begin();
    auto io = outputs.begin();
    while( it < targets.end() )
    {
        if( it->compare( *io ) == 0 )
        ++it;
        ++io;
    }

    Confusion c( targ, outp );
    Evaluation e( c );

    c.print();
    e.print();

}


}  // namespace score
