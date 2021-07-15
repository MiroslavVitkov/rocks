#include "task.h"

#include "label.h"

#include <future>
#include <thread>
#include <vector>


namespace task
{


void run_async( std::promise< std::vector< label::Num > > & p
              , const model::Base & m
              , const std::vector< dat::Spectrum > & spectra )
{
    std::vector< label::Num > ret;
    for( const auto & s : spectra )
    {
        ret.push_back( m.predict( s ) );
    }
    p.set_value( ret );
}


Task::Task( const model::Base & m, const std::vector< dat::Spectrum > & s )
        : _p{ }
        , _t{ run_async, std::ref( _p ), std::ref( m ), std::ref( s ) }
{
}


std::vector< label::Num > Task::get()
{
    _t.join();
    return _p.get_future().get();
}


// 1. Despite all this effort, moving a Task object
// (for example by a vector rellocating its storage)
// causes a weird crash.
// For now the workaround is to allocate enough memory in the container.
// 2. The threads bog down the machine, implement limit on their number.
Task::Task( Task && t )
    : _p{ std::move( t._p ) }
{
    _t.swap( t._t );
}


}  // namespace task
