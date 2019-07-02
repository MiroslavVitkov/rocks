#include "plot.h"

#include <QApplication>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <qwt_plot_renderer.h>

#include <string>


namespace plot
{


void plot( const dat::Spectrum & s, const std::string & title )
{
    // TODO: try #include <qwt_plot_spectrogram.h>
    int argc {};
    char ** argv {};
    QApplication a( argc, argv );

    QwtPlot plot;
    plot.setTitle( title.c_str() );
    plot.setCanvasBackground( Qt::white );
    plot.setAxisScale( QwtPlot::yLeft, -1500.0, 15000.0);
    plot.setAxisTitle(QwtPlot::xBottom, "wavelength, nm");
    plot.setAxisTitle(QwtPlot::yLeft, "intensity");

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->attach( &plot );

    QwtPlotCurve *curve = new QwtPlotCurve();
    curve->setPen( Qt::blue, 1 );
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QPolygonF points;
    auto itx = s._x.cbegin();
    auto ity = s._y.cbegin();
    while( itx != s._x.cend() )
    {
        points << QPointF( *itx++, *ity++ );
    }
    curve->setSamples( points );

    curve->attach( &plot );

    const QSize fullscreen{ 1920, 1080 - 25 };
    plot.resize( fullscreen.width(), fullscreen.height() );
    plot.show();

    QString fileName = "/tmp/bode.pdf";
    QwtPlotRenderer renderer;
    plot.replot();
    renderer.renderDocument( & plot,  fileName, fullscreen );

    a.exec();
}


}  // namespace plot
