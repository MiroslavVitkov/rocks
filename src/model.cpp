#include "model.h"

#include "dim.h"
#include "label.h"
#include "print.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/dnn.h>
#include <dlib/matrix.h>
#include <dlib/memory_manager.h>
#include <dlib/statistics.h>
#include <dlib/svm_threaded.h>
#endif

#include <algorithm>
#include <cassert>
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

    std::vector< label::Num > labels;
    labels.reserve( size );

    dat::apply( [ & ] ( label::Num l, const dat::Spectrum & )
        {
            labels.push_back( l );
        }
             , d );
    assert( labels.size() == size );

    return labels;
}


#ifdef CMAKE_USE_DLIB
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
    for( unsigned i {}; i < dat::Spectrum::_num_points; ++i )
    {
        assert( abs( s._y[ i ] - sam( i ) ) < 1e-9 );
    }

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
                if( d.first.empty() )
                {
                    return {};
                }

                Flattened all;
                std::vector< label::Num > i( d.first.size(), 0 );
                dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                {
                    addto( all, l, s );
                }         , d );

                Trainer trainer;
                trainer.set_num_threads( 10 );
                trainer.set_c( 1e0 );

                const Classifier svm{ trainer.train( all.first, all.second ) };
                return svm;
            } () }
    {
    }


    label::Num predict( const dat::Spectrum & test ) const
    {
        const auto s = to_dlib_sample( test );
        const auto ret = _svm.predict( s );
        return ret.first;  // what is ret.second?
    }


private:
    void addto( Flattened & f, label::Num l, const dat::Spectrum & s )
    {
        f.first.push_back( to_dlib_sample( s ) );
        f.second.push_back( l );
    }


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
#endif // CMAKE_USE_DLIB


#ifdef CMAKE_USE_SHARK
shark::RealVector to_shark_vector( const dat::Spectrum & s )
{
    return { s._y.cbegin(), s._y.cend() };
}


shark::ClassificationDataset to_shark_dataset( const dat::Dataset & d )
{
    if( d.first.empty() )
    {
        return {};
    }

    std::vector< shark::RealVector > inputs;
    std::vector< label::Num > labels;
    dat::apply( [&] ( label::Num l, const dat::Spectrum & s )
    {
        inputs.push_back( to_shark_vector( s ) );
        labels.push_back( static_cast< label::Num >( l ) );
    }
              , d );

    return shark::createLabeledDataFromRange( inputs, labels );
}


// ID of the commit with Andres' implementation: a8410b05.
auto train_forest_model( const shark::ClassificationDataset & dataset
                       , unsigned num_trees
                       )
{
    shark::RFClassifier< label::Num > model;
    shark::RFTrainer< label::Num > trainer;

    if( dataset.empty() )
    {
        return model;
    }

    trainer.setNTrees( num_trees );
    trainer.train( model, dataset );

    return model;
}


Forest::Forest( const dat::Dataset & d )
    : _model{ train_forest_model( to_shark_dataset( d ), _num_trees ) }
{
}


Forest::Forest( const shark::ClassificationDataset & d )
    : _model{ train_forest_model( d, _num_trees ) }
{

}


label::Num Forest::predict( const dat::Spectrum & s ) const
{
    return predict( to_shark_vector( s ) );
}


label::Num Forest::predict( const shark::RealVector & v ) const
{
    label::Num p;
    _model.eval( v, p );
    return p;
}
#endif  // CMAKE_USE_SHARK


const std::vector< std::string > ALL_MODELS{ "chance"
#ifdef CMAKE_USE_DLIB
                                           , "cor"
                                           , "svm"
#ifdef CMAKE_USE_OPENCV
                                           , "lda"
                                           , "pca"
#endif
                                           , "nn"
#endif
#ifdef CMAKE_USE_SHARK
                                           , "forest"
#endif
                                           };


}  // namespace model
