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
    for( const auto & kv : d.first )
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


int RandomChance::predict( const io::Spectrum & ) const
{
    std::random_device generator;
    std::uniform_real_distribution<double> distribution( 0, 1 );
    const auto rand = distribution( generator );

    // Comulutive probability distribution.
    double total {};
    for( const auto & kv : _probs )
    {
        total += kv.second;
        if( rand <= total )
        {
            return kv.first;
        }
    }

    throw Exception{ "Logical error: falling off RandomChance::predict()." };
}


Correlation::Correlation( const io::Dataset & d )
    : _training_set{ d }
{
}


// This function finds the highest correlation coefficient.
// It does not compute a correlation matrix.
int Correlation::predict( const io::Spectrum & test ) const
{
    const std::vector<double> vtest{ test._y.begin(), test._y.end() };

    double max {};
    int best {};
    io::walk( _training_set, [ & ] ( int l, const io::Spectrum & s )
         {
            std::vector<double> vtrain{ s._y.begin(), s._y.end() };
            const auto r_xy = dlib::correlation( vtrain, vtest );

            // Accuracy before introducing the modulo operation: 98.1468.
            // Accuracy after: 97.9687.
            const auto a = std::abs( r_xy );

            if( a > max )
            {
                max = a;
                best = l;
            }
         } );

    return best;
}


}  // namespace model
