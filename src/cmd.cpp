#include "cmd.h"

#include "dim.h"
#include "io.h"
#include "label.h"
#include "model.h"
#include "pre.h"
#include "print.h"
#include "score.h"

#include <shark/Data/Csv.h>
#include <shark/Algorithms/Trainers/LDA.h>
#include <shark/ObjectiveFunctions/Loss/ZeroOneLoss.h>

#include <future>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

shark::ClassificationDataset data;
namespace cmd
{


RunModel::RunModel( const std::string & data_dir
                  , const std::string & model_name )
    : _data_dir{ data_dir }
    , _model_name{ model_name }
{
}


void run_async( std::promise< label::Num > & p
              , const model::Model & m
              , const dat::Spectrum & s )
{
    const auto ret = m.predict( s );
    p.set_value( ret );
}


struct Task
{
    Task( const model::Model & m, const dat::Spectrum & s )
        :  _p{ }
        , _t{ run_async, std::ref( _p ), std::ref( m ), std::ref( s ) }
    {
    }


    label::Num get()
    {
        _t.join();
        return _p.get_future().get();
    }


    // 1. Despite all this effort, moving a Task object
    // (for example by a vector rellocating its storage)
    // causes a weird crash.
    // For now the workaround is to allocate enough memory in the container.
    // 2. The threads bog down the machine, implement limit on their number.
    Task( Task && t )
        : _p{ std::move( t._p ) }
    {
        _t.swap( t._t );
    }
    Task( const Task & ) = delete;
    ~Task() = default;
    Task& operator=( Task&& other) = delete;
    Task& operator=(const Task& other) = delete;


private:
    std::promise< label::Num > _p;
    std::thread _t;
};


void RunModel::execute()
{


    // Obtain the dataset.
    // When encoding, assume the test label is represented in the training set.
    print::info( "Reading the dataset." );
    auto raw = io::read( _data_dir, 2 );
    auto traintest = dat::split( raw );
    const auto train = dat::encode( traintest.first );
    const auto test = dat::encode( traintest.second, train.second );

    // Train the model.
    print::info( "Training a " + _model_name + " model." );
    const auto m = model::create( _model_name, train );

    // Evaluate the test set.
    std::vector< label::Num > ground_truth;
    std::vector< label::Num > predicted;
    std::vector< Task > tasks;
    const auto count = dat::count( test );
    ground_truth.reserve( count );
    predicted.reserve( count );
    tasks.reserve( count );

    print::info( "Evaluating the test set." );
    dat::apply( [ & ] ( int l, const dat::Spectrum & s )
        {
            ground_truth.push_back( l );
            tasks.emplace_back( * m, s );
        }
              , test );

    for( auto & t : tasks )
    {
        predicted.push_back( t.get() );
    }

    // Report.
    print::info( "Calculating confusion matrix." );
    const auto conf = score::calc_confusion( ground_truth, predicted );

    std::cout << "Confusion matrix, rows - ground truth, columns - prediction.\n"
                 "Labels: " << train.second << '\n'
              << conf
              << "\naccuracy: " << score::accuracy( conf ) << '\n';
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
