#include "dat.h"

#include <cassert>
#include <random>


namespace dat
{


void append( std::vector< Spectrum > & v1, const std::vector< Spectrum > & v2 )
{
    v1.insert( v1.end(), v2.cbegin(), v2.cend() );
}


std::pair< DataRaw, DataRaw > split( const DataRaw & d, double ratio )
{
    assert( ratio > 0 && ratio < 1 );

    std::default_random_engine generator;
    std::uniform_real_distribution distribution( 0., 1. );

    DataRaw train, test;

    for( const auto & kv : d )
    {
        const auto & label{ kv.first };
        for( const auto & datapoint : kv.second )
        {
            if( distribution( generator ) < ratio )
            {
                train[ label ].push_back( datapoint );
            }
            else
            {
                test[ label ].push_back( datapoint );
            }
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


void mutate( std::function< void ( const label::Raw &, Spectrum & ) > f
           , DataRaw & d )
{
    for( auto & label_vector : d )
    {
        for( auto & s : label_vector.second )
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


size_t count( const dat::DataRaw & d )
{
    size_t total {};
    dat::apply( [ & total ] ( const std::string &, const dat::Spectrum & )
        { ++total; }, d );
    return total;
}


}  // namespace dat
