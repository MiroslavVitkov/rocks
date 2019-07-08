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
    // When encoding, assume the test label is represented in the training set.
    auto raw = io::read( _data_dir, 2 );
    auto traintest = dat::split( raw );
    const auto train = dat::encode( traintest.first );
    const auto test = dat::encode( traintest.second, train.second );

    // Train the model.
    const auto m = model::create( _model_name, train );

    // Evaluate the test set.
    std::vector<int> ground_truth;
    std::vector<dat::Spectrum> flattened;
    dat::apply( [ & ] ( int l, const dat::Spectrum & s )
        {
            ground_truth.push_back( l );
            flattened.push_back( s );
        }
                          , test );
    const std::vector predicted( flattened.size(), m->predict( flattened ) );

    // Report.
    const auto conf = score::calc_confusion( ground_truth, predicted );

    std::cout << "Confusion matrix, rows - ground truth, columns - prediction.\n"
                 "Labels: " << train.second << '\n'
              << conf;
}


ReportOutliers::ReportOutliers( const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void ReportOutliers::execute()
{
    auto spectra = io::read( _data_dir );

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

    const auto dataset = dat::encode( spectra );

#define WHICH 4
#if WHICH == 0
    plot::plot( score::find_worst( [ & ] ( const io::Spectrum & s )
        {
            if( std::find( s._y.cbegin(), s._y.cend(), most_negative ) != s._y.cend() )
            {
                return -1;
            }

            return 0;
        }
                                 , dataset )
              , "Highest negative amplitude." );
#elif WHICH == 1

    const auto neg_energy = [] ( double init, double intensity )
    {
        if( intensity < 0 )
        {
            init += intensity * intensity;
        }
        return init;
    };

    plot::plot( score::find_worst( [ & ] ( const io::Spectrum & s )
        {
            return - std::accumulate( s._y.cbegin()
                                    , s._y.cend()
                                    , 0.0
                                    , neg_energy );
        }
                                 , dataset )
              , "Highest negative energy." );
#elif WHICH == 2
    plot::plot( score::find_worst( [ & ] ( const io::Spectrum & s )
        {
            return std::count_if( s._y.cbegin()
                                , s._y.cend()
                                , [] ( double d )
                {
                    return d < 0;
                }
                                );
        }
                                 , dataset )
              , "Highest number of negatives." );
#elif WHICH == 3
    const auto dat = io::read( _data_dir, 2 );
    io::apply( [] ( const std::string & label, const io::Spectrum)
    { std::cout << label << '\n'; }, dat );
#endif
}


}  // namespace cmd
