#include "task.h"

#include <future>
#include <thread>


namespace task
{


void run_async( std::promise< label::Num > & p
              , const model::Model & m
              , const std::vector< dat::Spectrum > & spectra )
{
    for( const auto & s : spectra )
    {

    }
    const auto ret = m.predict( s );
    p.set_value( ret );
}


Task::Task( const model::Model & m, const std::vector< dat::Spectrum > & s )
        : _p{ }
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




}  // namespace task
