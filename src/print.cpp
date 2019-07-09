#include "print.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>


namespace print
{


using Clock = std::chrono::system_clock;


void info( const char * s )
{
    const auto now_seconds = Clock::to_time_t( Clock::now() );
    const auto now_calendar = std::localtime( & now_seconds );
    std::cout << std::put_time( now_calendar, "%c" ) << ": " << s << std::endl;
}


}  // namespace print
