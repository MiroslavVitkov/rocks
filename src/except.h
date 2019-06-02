#ifndef EXCEPT_H_
#define EXCEPT_H_


// In this file: the project throws this exception.


#include <stdexcept>
#include <string>


struct Exception : public std::runtime_error
{
    Exception( const std::string & msg )
        : std::runtime_error( msg ) { }
};


#endif // defined(EXCEPT_H_)
