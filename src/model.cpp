#include "model.h"

#include "label.h"

#include <dlib/memory_manager.h>
#include <dlib/statistics.h>
#include <dlib/svm_threaded.h>

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


label::Num RandomChance::predict( const dat::Spectrum & ) const
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


std::vector< label::Num > construct_labels( const dat::Dataset & d )
{
    const auto size = count( d );

    std::vector< int > labels;
    labels.reserve( size );

    dat::apply( [ & ] ( int l, const dat::Spectrum & )
        {
            labels.push_back( l );
        }
             , d );
    assert( labels.size() == size );

    return labels;
}


Correlation::Correlation( const dat::Dataset & d )
    : _training_set{ d }
    , _labels{ construct_labels( d ) }
{
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
             , train );

    return ret;
}


label::Num Correlation::predict( const dat::Spectrum & test ) const
{
    const auto row = compute_correlation_row( _training_set, test );

    const auto m = std::max_element( row.cbegin(), row.cend() );
    assert( m != row.cend() );

    const auto index = std::distance( row.begin(), m );
    assert( static_cast<size_t>( index ) < _labels.size() );
    const auto l = _labels[ static_cast<size_t>( index ) ];

    return l;
}


SVM::SVM( const dat::Dataset & d )
    : _training_set{ [ & ] ()
        {
            Flattened ret;

            dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                {
                    const auto p = reinterpret_cast< const double ( * )
                                                   [ dat::Spectrum::_num_points ] >
                                   ( s._y.data() );
                    assert( p );

                    ret.first.emplace_back( * p );
                    ret.second.push_back( l );
                }     , d );

            using sample_type = dlib::matrix< double, dat::Spectrum::_num_points, 1 >;
            using kernel_type = dlib::linear_kernel< sample_type >;
            dlib::svm_multiclass_linear_trainer< kernel_type, label::Num > svm_trainer;
            const auto svm = svm_trainer.train( ret.first, ret.second );

            return ret;
        } () }
{
}


label::Num SVM::predict( const dat::Spectrum & test ) const
{

//    Sample s{ test._y.cbegin(), test._y.cend() };
//    const auto ret = svm.predict( test );
//    return ret;
    (void)test;
    return 0;
}


}  // namespace model
