#include "plot.h"

#include <QApplication>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>

#include <string>


namespace plot
{


void plot( const io::Spectrum & s, const std::string & title )
{
    int argc {};
    char ** argv {};
    QApplication a( argc, argv );

    QwtPlot plot;
    plot.setTitle( title.c_str() );
    plot.setCanvasBackground( Qt::white );
    plot.setAxisScale( QwtPlot::yLeft, -1500.0, 15000.0);
    plot.insertLegend( new QwtLegend() );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( &plot );

    QwtPlotCurve *curve = new QwtPlotCurve();
//    curve->setTitle( "Pixel Count" );
    curve->setPen( Qt::blue, 1 );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

//    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
//        QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
//    curve->setSymbol( symbol );

    QPolygonF points;
    auto itx = s._x.cbegin();
    auto ity = s._y.cbegin();
    while( itx != s._x.cend() )
    {
        points << QPointF( *itx++, *ity++ );
    }
    curve->setSamples( points );

    curve->attach( &plot );

    plot.resize( 1920, 1080 - 25 );
    plot.show();

    a.exec();
}



}  // namespace plot
