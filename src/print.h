#ifndef PRINT_H_
#define PRINT_H_


// In this file: logging to stdout and stderr.


#include <string>


namespace print
{


void info( const char * );
inline void info( const std::string & s ) { info( s.c_str() ); }


}  // namespace print


#endif // defined( PRINT_H_ )
