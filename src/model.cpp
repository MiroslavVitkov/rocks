#include "model.h"


#include <dlib/matrix.h>
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
    // The correlation matrix has one row per test sample
    // and one column per training sample.
    const auto rows = 1;
    const auto cols = [ & ] (  )
    {
        unsigned total {};
        io::walk( _training_set
                , [ & ] ( int, const io::Spectrum & )
                { ++total; } );
        return total;
    } ();
    dlib::matrix< double > correlations( rows, cols );
    std::vector< int > labels;
    labels.reserve( cols );

    // Compute the matrix.
    auto it = correlations.begin();
    const std::vector<double> vtest{ test._y.begin(), test._y.end() };
    io::walk( _training_set, [ & ] ( int l, const io::Spectrum & s )
     {
        std::vector<double> vtrain{ s._y.begin(), s._y.end() };
        const auto r_xy = dlib::correlation( vtrain, vtest );

        *it++ = r_xy;
        labels.push_back( l );
     } );

    // Interpret it.
    // For now, just find the global maximum.
    // Should we apply the modulo operation to values?
    // Accuracy without modulo operation: 98.1468%, with: 97.9687%.
    const auto max = std::max_element( correlations.begin()
                                     , correlations.end() );
    assert( max != correlations.end() );
    const auto index = max - correlations.begin();

    return labels[ index ];
}


}  // namespace model
