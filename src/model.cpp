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


RandomChance::RandomChance( const dat::Dataset & d )
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


int RandomChance::predict( const std::vector< dat::Spectrum > & ) const
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


Correlation::Correlation( const dat::Dataset & d )
    : _training_set{ d }
{
}


unsigned num_elements( const dat::Dataset & d )
{
    unsigned total {};
    dat::apply( [ &total ] ( int, const dat::Spectrum & ) { ++total; }, d );
    return total;
}


std::vector<double> compute_correlation_row( const dat::Dataset & train
                                           , const dat::Spectrum & test )
{
    std::vector<double> ret;

    const std::vector<double> vtest{ test._y.begin(), test._y.end() };
    dat::apply( [ &vtest, &ret ] ( int, const dat::Spectrum & s)
        {
            std::vector<double> vtrain{ s._y.begin(), s._y.end() };
            const auto r_xy = dlib::correlation( vtrain, vtest );
            ret.push_back( r_xy );
        }
             , train);

    return ret;
}


// This function finds the highest correlation coefficient.
// It does not compute a correlation matrix.
int Correlation::predict( const std::vector< dat::Spectrum > & test ) const
{
    // The correlation matrix.
    const auto rows = static_cast<long>( test.size() );
    const auto cols = num_elements( _training_set );
    dlib::matrix< double > correlations( rows, cols );

    // High correlation would meant that the test sample
    // has the same label as the respective training sample.
    std::vector< int > labels;
    labels.reserve( cols );
    dat::apply( [ & ] ( int l, const dat::Spectrum & )
        {
            labels.push_back( l );
        }
             , _training_set );
    assert( labels.size() == cols );

    // Compute the matrix.
    std::vector<int> predictions;
    for( unsigned i {}; i < test.size(); ++i )
    {
        const auto row = compute_correlation_row( _training_set, test[ i ] );
        std::copy( row.cbegin(), row.cend(), & correlations(i, 0) );

        // Interpret it.
        // Accuracy without modulo operation: 98.1468%, with: 97.9687%.
        const auto m = std::max_element( row.cbegin(), row.cend() );
        assert( m != row.end() );
        const auto index = std::distance( row.begin(), m );
        assert( static_cast<size_t>( index ) < labels.size() );
        predictions.push_back( labels[ static_cast<size_t>( index ) ] );
    }

    return predictions.front();

#if 0
    // Policy similar to World of Darkness.
    // Doesn't work well.
    const double treshold = 0.9;
    std::unordered_map< int, unsigned > counts;
    for( unsigned i {}; i < correlations.size(); ++i )
    {
        if( correlations( 0, i ) >= treshold )
        {
            ++counts[ labels[ i ] ];
        }
    }

    if( counts.empty() )
    {
        // oh shit
        std::cout << "No class correlates strongly enough. Skipping.\n";
        return 0;
    }

    const auto predicted = std::max_element( counts.begin(), counts.end(),
        [] ( const std::pair<int, unsigned> & p1
           , const std::pair<int, unsigned> & p2)
        {
                return p1.second < p2.second;
        } );
    assert( predicted != counts.end() );

    return predicted->first;
#endif
#if 0
    // Sum of correlation coefficients per classes.
    std::unordered_map< int, unsigned > sums;
    for( unsigned i {}; i < correlations.size(); ++i )
    {
        sums[ labels[ i ] ] += std::abs( correlations( 0, i ) );
    }

    const auto predicted = std::max_element( sums.begin(), sums.end(),
        [] ( const std::pair<int, unsigned> & p1
           , const std::pair<int, unsigned> & p2)
        {
                return p1.second < p2.second;
        } );
    assert( predicted != sums.end() );

    return predicted->first;
#endif

}


}  // namespace model
