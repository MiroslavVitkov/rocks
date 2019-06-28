#ifndef PLOT_H_
#define PLOT_H_


// In this file: plotting capabilities


#include "io.h"


namespace plot
{


void plot( const io::Spectrum &, const std::string & title = "" );


}  // namespace plot


#endif // defined( PLOT_H_ )
