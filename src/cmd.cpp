#include "cmd.h"

#include "io.h"
#include "model.h"
#include "plot.h"
#include "score.h"

#include <iostream>
#include <numeric>
#include <vector>


namespace cmd
{


RunModel::RunModel( const std::string & data_dir
                  , const std::string & model_name )
    : _data_dir{ data_dir }
    , _model_name{ model_name }
{
}


void RunModel::execute()
{
    // Obtain the dataset.
    auto raw = io::read( _data_dir );
    const auto encoded = io::encode( raw );
    const auto traintest = score::train_test_split( encoded );

    // Train the model.
    const auto m = model::create( _model_name, traintest.first );

    // Evaluate the test set.
    std::vector<int> targets;
    std::vector<io::Spectrum> flattened;
    io::apply( [ & ] ( int l, const io::Spectrum & s )
        {
            targets.push_back( l );
            flattened.push_back( s );
        }
             , traintest.second );
    const std::vector outputs( flattened.size(), m->predict( flattened ) );

    // Report.
    score::evaluate_and_print( targets, outputs );
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
    const io::Spectrum * worst {};

    io::apply( [ & ] ( const std::string &, const io::Spectrum & s )
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
              << sum_intensity / ( num_files * io::Spectrum::_num_points )
              << ".\nThe global count of negative values is " << num_negatives
              << ", which is "
              << num_negatives / ( num_files * io::Spectrum::_num_points * 1.0 )
              << " of all datapoints.\n"
              << "The mean of all negative values is "
              << sum_negatives / num_negatives
              << ".\nThe most extreme negative value is " << most_negative
              << ".\n";

    plot::plot( * worst );
}


}  // namespace cmd
