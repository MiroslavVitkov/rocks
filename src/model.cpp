#include "model.h"

#include "dim.h"
#include "label.h"
#include "print.h"

#include <andres/ml/decision-trees.hxx>

#include <dlib/dnn.h>
#include <dlib/matrix.h>
#include <dlib/memory_manager.h>
#include <dlib/statistics.h>
#include <dlib/svm_threaded.h>

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


#if 0
                const auto validate_prop = 0.3;
                Flattened train, validate;
#endif
                Flattened all;
                std::vector< unsigned > i( d.first.size(), 0 );
                dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                {
#if 0
                    const auto ll = static_cast< unsigned >( l );
                    if( i[ ll ]++ < d.first.at( l ).size() * validate_prop )
                    {
                        addto( validate, l, s );
                    }
                    else
                    {
                        addto( train, l, s );
                    }
#endif
                    addto( all, l, s );
                }         , d );

#if 0
                print::info( "Performing grid search for coefficient c." );
                const double C[] = { 1e-5, 1e-4, 1e-3, 1e-2, 1e-1, 1e0
                                   , 1e1, 1e2, 1e3, 1e4, 1e5 };
                std::vector< double > accuracy;
                for( const auto c : C )
                {
                    Trainer trainer;
                    trainer.set_c( c );
                    trainer.set_num_threads( 10 );
                    const Classifier svm = trainer.train( train.first, train.second );

                    // Test.
                    unsigned correct{}, total{};
                    for( unsigned i{}; i < validate.first.size(); ++i )
                    {
                        const auto pred = svm.predict( validate.first[ i ] );
                        const auto truth = validate.second[ i ];
                        if( truth == pred.first )
                        {
                            ++correct;
                        }
                        ++total;
                    }
                    const auto a = 1.0 * correct / total;
                    accuracy.push_back( a );
                    std::cout << "c = " << c << ", accuracy = " << a << std::endl;
                }
#endif

                Trainer trainer;
                trainer.set_num_threads( 10 );
                trainer.set_c( 1e0 );

                const Classifier svm = trainer.train( all.first, all.second );
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


struct LDAandSVM::Impl
{
    using Sample = dlib::matrix< dat::Compressed::value_type
                               , dat::Compressed::_num_points
                               , 1 >;
    using Kernel = dlib::linear_kernel< Sample >;
    using Classifier = dlib::multiclass_linear_decision_function< Kernel
                                                                , label::Num >;
    using Trainer = dlib::svm_multiclass_linear_trainer< Kernel
                                                       , label::Num >;


    Impl( const dat::Dataset & d )
        : _lda{ d }
        , _svm{ [ & ] () -> Classifier
            {
                using Flattened = std::pair< std::vector< Sample >
                                           , std::vector< label::Num > >;

                Flattened fl;

                dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                    {
                        const auto lda{ _lda( s ) };

                        Sample sample;
                        unsigned row {};
                        for( const auto a : lda._y )
                        {
                            sample( row++ ) = a;
                        }

                        fl.first.emplace_back( sample );
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
        const auto compressed = _lda( test );

        Sample m;

        unsigned row {};
        for( const auto & a : compressed._y )
        {
            m( row++ ) = a;
        }

        const auto ret = _svm.predict( m );

        return ret.first;  // what is ret.second?
    }


private:
    const dim::LDA _lda;
    const Classifier _svm;
};

LDAandSVM::LDAandSVM( const dat::Dataset & d )
    : _impl{ std::make_unique< Impl >( d ) }
{
}


label::Num LDAandSVM::predict( const dat::Spectrum & test ) const
{
    return _impl->predict( test );
}


LDAandSVM::~LDAandSVM()
{
}


struct PCAandSVM::Impl
{
    using Sample = dlib::matrix< dat::Compressed::value_type
                               , dat::Compressed::_num_points
                               , 1 >;
    using Kernel = dlib::linear_kernel< Sample >;
    using Classifier = dlib::multiclass_linear_decision_function< Kernel
                                                                , label::Num >;
    using Trainer = dlib::svm_multiclass_linear_trainer< Kernel
                                                       , label::Num >;


    Impl( const dat::Dataset & d )
        : _pca{ d }
        , _svm{ [ & ] () -> Classifier
            {
                using Flattened = std::pair< std::vector< Sample >
                                           , std::vector< label::Num > >;

                Flattened fl;

                dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
                    {
                        const auto compressed{ _pca( s ) };

                        Sample sample;
                        unsigned row {};
                        //std::copy( pca._y.cbegin(), pca._y.cend(), sample( row++ ) );
                        for( const auto a : compressed._y )
                        {
                            sample( row++ ) = a;
                        }

                        fl.first.emplace_back( sample );
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
        const auto compressed = _pca( test );
        Sample m;
        //std::copy( compressed._y.cbegin(), compressed._y.cend(), m.begin() );
        unsigned row {};
        for( const auto a : compressed._y )
        {
            m( row++ ) = a;
        }
        const auto ret = _svm.predict( m );
        return ret.first;  // what is ret.second?
    }


private:
    const dim::PCA _pca;
    const Classifier _svm;
};


PCAandSVM::PCAandSVM( const dat::Dataset & d )
    : _impl{ std::make_unique< Impl >( d ) }
{
}


label::Num PCAandSVM::predict( const dat::Spectrum & test ) const
{
    return _impl->predict( test );
}


PCAandSVM::~PCAandSVM()
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


struct Forest::Impl
{
    using Feature = double;
    using Label = label::Num;
    using Probability = double;


    Impl( const dat::Dataset & d )
    {
        const auto num_samples = dat::count( d );
        if( ! num_samples )
        {
            return;
        }

        const auto num_features = dat::Spectrum::_num_points;
        andres::Matrix< Feature > f( num_samples, num_features );
        auto f_it = f.begin();

        andres::Vector< Label > l( num_samples );
        auto l_it = l.begin();

        dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
            {
                std::copy( s._y.cbegin(), s._y.cend(), f_it );
                f_it += num_features;

                * l_it = l;
                ++l_it;
            }     , d );

        _model.learn( f, l );
    }


    label::Num predict( const dat::Spectrum & s ) const
    {
        andres::Matrix< Feature > f( 1, dat::Spectrum::_num_points );
        std::copy( s._y.cbegin(), s._y.cend(), f.begin() );

        andres::Matrix< Probability > p( 1, 6 );
        _model.predict( f, p );

        const auto it = std::max_element( p.begin(), p.end() );
        const auto predicted = it - p.begin();

        return static_cast< label::Num >( predicted );
    }

    andres::ml::DecisionForest< Feature, Label, Probability > _model;
};


Forest::Forest( const dat::Dataset & d )
    : _impl{ std::make_unique< Impl >( d ) }
{
}


label::Num Forest::predict( const dat::Spectrum & s ) const
{
    return _impl->predict( s );
}


Forest::~Forest()
{
}


}  // namespace model
