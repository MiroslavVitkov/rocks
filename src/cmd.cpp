#include "cmd.h"

#include "io.h"
#include "label.h"
#include "model.h"
#include "pre.h"
#include "print.h"
#include "score.h"
#include "task.h"

#include <iostream>
#include <numeric>
#include <vector>


namespace cmd
{


RunModel::RunModel( const std::string & data_dir
                  , const std::string & model_name
                  , unsigned labels_depth
                  , const std::vector< std::string > & preprocessing
                  )
    : _data_dir{ data_dir }
    , _model_name{ model_name }
    , _labels_depth{ labels_depth }
    , _preprocessing{ preprocessing }
{
}


// Preprocess the dataset e.g. dimensionality reduction.
std::pair< dat::Dataset, dat::Dataset >
RunModel::preprocess_dataset()
{
    print::info( std::string("Reading dataset '") + _data_dir
               + "' at labels depth " + std::to_string(_labels_depth) );
    auto raw{ io::read( _data_dir, _labels_depth ) };
    const auto encoded{ dat::encode( std::move( raw ) ) };

    // Pipeline steps executed in order on cmdline.
    // Each operates on the output of the previous one.
    auto tmp{ encoded };
    for( const auto & name : _preprocessing )
    {
        const auto node{ pre::create( name, encoded ) };
        tmp = (*node)( tmp );
    }

    // Perform holdout split.
    auto processed{ encoded };
    const auto traintest{ dat::split( std::move( processed ) ) };
    return traintest;
}


void evaluate( const dat::Dataset & test
             , const model::Model & m
             )
{
    print::info( "Evaluating the test set." );
    std::vector< label::Num > ground_truth;
    std::vector< label::Num > predicted;
    for( const auto & kv : test.first )
    {
        for( const auto & s : kv.second )
        {
            ground_truth.push_back( kv.first );
            predicted.push_back( m.predict( s ) );
        }
    }

    // Reduce to head labels.
    const auto headonly{ test.second.headonly() };
    const auto gt{ label::headonly_recode( ground_truth, test.second, headonly) };
    const auto pr{ label::headonly_recode( predicted, test.second, headonly) };

#ifdef CMAKE_USE_DLIB
    print::info( "Calculating confusion matrix." );
    const auto conf = score::calc_confusion( gt, pr );

    std::cout << "Confusion matrix, rows - ground truth, columns - prediction.\n"
                 "Labels: " << test.second << '\n'
              << conf
              << "\naccuracy: " << score::accuracy( conf ) << '\n';
#else
    print::info( "Accuracy evaluadion disabled because dlib is not used." );
#endif  // CMAKE_USE_DLIB
}


void RunModel::execute()
{
    const auto traintest{ preprocess_dataset() };

    print::info( "Training a " + _model_name + " model." );
    const auto m{ model::create( _model_name, traintest.first ) };

    evaluate( traintest.second, * m );
}


RunAllModels::RunAllModels( const std::string & data_dir
                          , unsigned labels_depth_max
                          )
    : _data_dir{ data_dir }
    , _labels_depth_max{ labels_depth_max }
{
}


void RunAllModels::execute()
{
    for( const auto & m : model::ALL_MODELS )
    {
        for( const auto & p : pre::ALL_PRE )
        {
            for( auto l{ static_cast< int >( _labels_depth_max ) }; l >= 0; --l )
            {
                RunModel( _data_dir
                        , m
                        , l
                        , std::vector< std::string >{ p }
                        );
            }
        }
    }
}


ReportOutliers::ReportOutliers( const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void ReportOutliers::execute()
{
    const auto spectra = io::read( _data_dir );

    // Measure.
    unsigned num_files {};
    long double sum_intensity {};
    unsigned num_negatives {};
    double sum_negatives {};
    double most_negative {};
    const dat::Spectrum * worst {};

    dat::apply( [ & ] ( const std::string &, const dat::Spectrum & s )
        {
            ++num_files;
            sum_intensity = std::accumulate( s._y.cbegin()
                                             , s._y.cend()
                                             , sum_intensity );

            for( const auto v : s._y )
            {
                if( v < 0 )
                {
                    ++num_negatives;
                    sum_negatives += v;
                    if( v < most_negative )
                    {
                        most_negative = v;
                        worst = & s;
                    }
                }
            }
        }
             , spectra );

    // Report.
    std::cout << "Dataset consists of " << num_files << " files.\n"
              << "The global micro average intensity is "
              << sum_intensity / ( num_files * dat::Spectrum::_num_points )
              << ".\nThe global count of negative values is " << num_negatives
              << ", which is "
              << num_negatives / ( num_files * dat::Spectrum::_num_points * 1.0 )
              << " of all datapoints.\n"
              << "The mean of all negative values is "
              << sum_negatives / num_negatives
              << ".\nThe most extreme negative value is " << most_negative
              << ".\n";
}


}  // namespace cmd
