#include "score.h"

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
                distribution( 0, kv.second.size() );
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


}  // namespace score
