#include "cmd.h"

#include "io.h"
#include "model.h"
#include "score.h"

// TODO: remove those includes, they are here for testing purposes.
#include <QApplication>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

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
    int argc = 7;
    char ** argv{};
    QApplication a( argc, argv );

    QwtPlot plot;
    plot.setTitle( "Plot Demo" );
    plot.setCanvasBackground( Qt::white );
    plot.setAxisScale( QwtPlot::yLeft, 0.0, 10.0);
    plot.insertLegend( new QwtLegend() );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( &plot );

    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setTitle( "Pixel Count" );
    curve->setPen( Qt::blue, 4 ),
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
    curve->setSymbol( symbol );

    QPolygonF points;
    points << QPointF( 0.0, 4.4 ) << QPointF( 1.0, 3.0 )
        << QPointF( 2.0, 4.5 ) << QPointF( 3.0, 6.8 )
        << QPointF( 4.0, 7.9 ) << QPointF( 5.0, 7.1 );
    curve->setSamples( points );

    curve->attach( &plot );

    plot.resize( 600, 400 );
    plot.show();

    a.exec();

    ////////////////////////////////

    const auto spectra = io::read( _data_dir );

    // Measure.
    unsigned num_files {};
    long double sum_intensity {};
    unsigned num_negatives {};
    double sum_negatives {};
    double most_negative {};

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
}


}  // namespace cmd
