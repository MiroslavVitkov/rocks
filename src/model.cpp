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


auto count_fequencies( const auto & d )
{
    std::unordered_map<int, double> ret;

    // Count instances of each class.
    unsigned datapoints {};
    for( const auto & kv : d.first )
    {
        ret.emplace( kv.first, kv.second.size() );
        datapoints += kv.second.size();
    }

    // Normalize values so that they sum up to 1.
    for( auto & kv : ret )
    {
        kv.second /= datapoints;
    }

    return ret;
}


RandomChance::RandomChance( const dat::Dataset & d )
    : _probs{ count_fequencies( d ) }
{
}


RandomChance::RandomChance( const dat::DatasetCompressed & d )
    : _probs{ count_fequencies( d ) }
{
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


struct SVM::Impl
{
    using Kernel = dlib::linear_kernel< dat::DlibSample >;
    using Classifier = dlib::multiclass_linear_decision_function< Kernel, label::Num >;
    using Trainer = dlib::svm_multiclass_linear_trainer< Kernel, label::Num >;


    Impl( const dat::Dataset & d )
        : _svm{ [ & ] () -> Classifier
            {
                if( d.first.empty() )
                {
                    return {};
                }

                dat::DlibFlattened all;
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
        const auto s = dat::to_dlib_sample( test );
        const auto ret = _svm.predict( s );
        return ret.first;  // what is ret.second?
    }


private:
    void addto( dat::DlibFlattened & f, label::Num l, const dat::Spectrum & s )
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
#endif // CMAKE_USE_DLIB


#ifdef CMAKE_USE_SHARK
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
#endif
#ifdef CMAKE_USE_SHARK
                                           , "forest"
#endif
                                           };


}  // namespace model
