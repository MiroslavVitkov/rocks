#include "score.h"

#include <evaluation.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <set>


namespace score
{


std::pair< io::Dataset, io::Dataset >
train_test_split( const io::Dataset & d
                , double test_proportion )
{
    io::Dataset train{ {}, d.second };
    io::Dataset test{ {}, d.second };
    for( const auto & kv : d.first )
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
                test.first[ kv.first ].push_back( kv.second[ i ] );
            }
            else
            {
                train.first[ kv.first ].push_back( kv.second[ i ] );
            }
        }
        assert( train.first[ kv.first ].size()
              + test.first[ kv.first ].size()
                == d.first.at( kv.first ).size() );
    }

    return { train, test };
}


void evaluate_and_print( const std::vector<int> & targets
                       , const std::vector<int> & outputs)
{
    Confusion c( targets, outputs );
    Evaluation e( c );

    c.print();
    e.print();
}


const io::Spectrum & find_worst( Comp c, const io::Dataset & d )
{
    using P = std::pair< int, const io::Spectrum & >;
    std::vector< P > scores;
    io::apply( [ & ] ( int, const io::Spectrum & s )
        {
            scores.emplace_back( c( s ), s );
        }
             , d );

    const auto & m = * std::max_element( scores.cbegin()
                                       , scores.cend()
                                       , [] ( const P & p1, const P & p2 )
        {
            int a = p1.first;
            int b = p2.first;
            return a > b;
            //return p1.first < p2.second;
        }    );

    return m.second;
}

}  // namespace score
