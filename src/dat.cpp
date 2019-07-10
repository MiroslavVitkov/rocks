#include "dat.h"

#include "except.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>


namespace dat
{


void append( std::vector< Spectrum > & v1, const std::vector< Spectrum > & v2 )
{
    v1.insert( v1.end(), v2.cbegin(), v2.cend() );
}


std::pair< DataRaw, DataRaw > split( const DataRaw & d )
{
    DataRaw train, test;

    std::set< label::RawShort > processed;
    for( const auto & kv : d )
    {
        const auto l = label::head( kv.first );

        if( processed.count( l ) )
        {
            append( train[ l ], kv.second );
        }
        else
        {
            append( test[ l ], kv.second );
            processed.emplace( l );
        }
    }

    return { train, test };
}


Dataset encode( DataRaw & raw )
{
    Dataset dt;

    for( auto & kv : raw )
    {
        dt.first.emplace( dt.second.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


Dataset encode( DataRaw & raw, const label::Codec & t )
{
    Dataset dt{ {}, t };

    for( auto & kv : raw )
    {
        dt.first.emplace( t.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


// The iteration order of unordered associative containers can only change
// when rehashing as a result of a mutating operation
// (as described in C++11 23.2.5/8).
void apply( std::function< void ( label::Num, const Spectrum & ) > f
          , const Dataset & d )
{
    for( const auto & label_vector : d.first )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void apply(std::function< void ( const label::Raw &, const Spectrum & ) > f
          , const DataRaw & d )
{
    for( const auto & label_vector : d )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


size_t count( const dat::Dataset & d )
{
    size_t total {};
    dat::apply( [ &total ] ( int, const dat::Spectrum & ) { ++total; }, d );
    return total;
}


}  // namespace dat
