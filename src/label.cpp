#include "label.h"

#include "except.h"

#include <cassert>
#include <set>


namespace label
{


Num Codec::encode( const RawShort & l )
{
    if( _encoding.count( l ) == 0 )
    {
        _encoding[ l ] = static_cast<int>( _encoding.size() );
        _reverse[ static_cast<int>(  _reverse.size() ) ] = l ;
    }

    return _encoding[ l ];
}


Num Codec::encode( const RawShort & l ) const
{
    if( _encoding.count( l ) == 0 )
    {
        throw Exception{ "Label encoding failed. "
                         "Label '" + l + "' not found." };
    }

    return _encoding.at( l );
}


const RawShort & Codec::decode( Num i ) const
{
    if( _reverse.count( i ) == 0 )
    {
        throw Exception{ "Label decoding failed. "
                         "Value " + std::to_string( i ) + " not found." };
    }

    return _reverse.at( i );
}


std::ostream & operator<<( std::ostream & s, const Codec & t )
{
    for( const auto & kv : t._encoding )
    {
        s << kv.first << ": " << kv.second << ", ";
    }

    return s;
}


std::string head( const std::string & full_label )
{
    assert( full_label[0] == '/' );
    const auto pos = full_label.find_first_of( '/', 1 );
    return full_label.substr( 0, pos );
}


}  // namespace label
