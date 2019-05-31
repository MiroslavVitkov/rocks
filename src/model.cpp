#include "model.h"

#include <dlib/statistics.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <vector>


namespace model
{


RandomChance::RandomChance( const io::Dataset & d )
{
    // Count instances of each class.
    unsigned datapoints {};
    for( const auto & kv : d )
    {
        _probs.emplace( kv.first, kv.second.size() );
        datapoints += kv.second.size();
    }

    // Normalize values so that they sum up to 1.
    for( auto & kv : _probs )
    {
        kv.second /= datapoints;
    }
}


io::Label RandomChance::predict( const io::Spectrum & ) const
{
    std::random_device generator;
    std::uniform_real_distribution<double> distribution( 0, 1 );
    const auto rand = distribution(generator);
    double total {};
    for( const auto & kv : _probs )
    {
        total += kv.second;
        if( rand <= total )
        {
            return kv.first;
        }
    }

    assert( false );
}


Correlation::Correlation( const io::Dataset & d )
    : _training_set{ d }
{
}


io::Label Correlation::predict( const io::Spectrum & test ) const
{
    std::vector<double> res;

    for( const auto & kv : _training_set )
    {
        for( const auto & datapoint : kv.second )
        {
            std::vector<double> t{ test._y.begin()
                                 , test._y.end() };
            std::vector<double> tr{ datapoint._y.begin()
                                  , datapoint._y.end() };
            const auto r = dlib::correlation( t, tr );
            res.push_back( r );
        }
    }

    const auto me = std::max_element( res.begin(), res.end() );
    return std::to_string( *me ) + ";" + std::to_string( me - res.begin() );

}


}  // namespace model
