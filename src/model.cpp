#include "model.h"

#include "label.h"

#include <dlib/dnn.h>
#include <dlib/matrix.h>
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


using Sample = dlib::matrix< double, dat::Spectrum::_num_points, 1 >;
using Flattened = std::pair< std::vector< Sample >
                           , std::vector< label::Num > >;


Sample to_dlib_sample( const dat::Spectrum & s )
{
    const auto p = reinterpret_cast< const double ( * )
                                   [ dat::Spectrum::_num_points ] >
                   ( s._y.data() );
    assert( p );

    Sample sam( * p );
    return sam;
}


struct SVM::Impl
{
    using Kernel = dlib::linear_kernel< Sample >;
    using Classifier = dlib::multiclass_linear_decision_function< Kernel, label::Num >;
    using Trainer = dlib::svm_multiclass_linear_trainer< Kernel, label::Num >;


    Impl( const dat::Dataset & d )
        : _svm{ [ & ] () -> Classifier
            {
                Flattened fl;

                dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                    {
                        fl.first.emplace_back( to_dlib_sample( s ) );
                        fl.second.push_back( l );
                    }     , d );

                if( fl.first.empty() )
                {
                    return {};
                }

                Trainer trainer;
                trainer.set_num_threads( 10 );

                const Classifier svm = trainer.train( fl.first, fl.second );
                return svm;
            } () }
    {
    }


    label::Num predict( const dat::Spectrum & test ) const
    {
        const auto s = to_dlib_sample( test );
        const auto ret = _svm.predict( s ); (void)ret;
        return ret.first;  // what is ret.second?
    }


    Impl( Impl && ) = default;
    Impl & operator=( Impl && ) = default;
    Impl( const Impl & ) = default;
    Impl & operator=( const Impl & ) = default;
    ~Impl() = default;


private:
    const Classifier _svm;
};


SVM::SVM( const dat::Dataset & d )
    : _impl{ std::make_unique< Impl >( d ) }
{
}


label::Num SVM::predict( const dat::Spectrum & test ) const
{
    return _impl->predict( test );
}


SVM::~SVM()
{
}

    using namespace dlib;
struct NN::Impl
{
    // Layers.
    using Input = dlib::input< Sample >;
    using Compute0 = dlib::softmax< dlib::fc< 100, Input > >;
    using Compute1 = dlib::softmax< dlib::fc< 50, Compute0 > >;
    using Output = dlib::loss_multiclass_log< dlib::fc< 6, Compute1 > >;

    using Net = Output;
    using Trainer = dlib::dnn_trainer< Net >;


    Impl( const dat::Dataset & d )
    {
        if( d.first.empty() )
        {
            return;
        }

        Trainer t{ _net };
        t.set_learning_rate( 1e-3 );
        t.set_min_learning_rate( 1e-6 );
        t.set_mini_batch_size( 30 );
        t.be_verbose();

        using Flattened = std::pair< std::vector< Sample >
                                   , std::vector< unsigned long > >;
        Flattened fl;
        dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
            {
                fl.first.emplace_back( to_dlib_sample( s ) );
                fl.second.push_back( static_cast< unsigned long >( l ) );
            }     , d );

        t.train( fl.first, fl.second );
        t.get_net();
    }


    label::Num predict( const dat::Spectrum & s ) const
    {
        std::vector< Sample > v{ to_dlib_sample( s ) }; (void)v;
        const auto predicted = _net( v );
        const auto p = predicted.front();
        return static_cast< label::Num >( p );
    }


    mutable Net _net {};
};


NN::NN( const dat::Dataset & d )
    : _impl{ std::make_unique< Impl >( d ) }
{
}


label::Num NN::predict( const dat::Spectrum & s ) const
{
    return _impl->predict( s );
}


NN::~NN()
{
}


}  // namespace model
