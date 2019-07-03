#include "dat.h"

#include "except.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>


namespace dat
{


/////////////////
// TODO: this, along with most other stuff, goes to dataset.cpp/h
#if 0
std::set<unsigned> pick_indices( size_t top )
{
    std::set<unsigned> indices;

    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned>
        distribution( 0, static_cast<unsigned>( top ) );

    while( indices.size() < 1 )
    {
        const auto rand = distribution( generator );
        indices.insert( rand );
    }

    return indices;
}


size_t pick_index( size_t top )
{
    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned>
        distribution( 0, static_cast<unsigned>( top ) );

    return distribution( generator );
}


std::pair< io::DataRaw, io::DataRaw >
split( io::DataRaw & d, double test_proportion )
{
    (void) test_proportion;  // hardcoded as 1 test folder for now

    // Choose one subfolder.
    const auto labels = [ & d ] ()
        {
            std::vector<std::string> ret;
            for( const auto & kv : d )
            {
                ret.emplace_back( kv.first );
            }
            return ret;
        } ();
    const auto label = labels[ pick_index( labels.size() ) ];

    // Copy all instances of each label.
    io::DataRaw train, test;
    for( auto & kv : d )
    {
        const auto short_label = [ & label ] ()
            {
                // Every label starts with '/'.
                const auto without_leading = label.substr( 1 );
                const auto pos = without_leading.find_first_of( '/' );
                return without_leading.substr( 0, pos );
            } ();

        if( kv.first == label )
        {
            test[ short_label ] = std::move( kv.second );
        }
        else
        {
            train[ short_label ] = std::move( kv.second );
        }
    }

    return { train, test };
}
#endif


size_t pick_index( size_t top )
{
    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned>
        distribution( 0, static_cast<unsigned>( top ) );

    return distribution( generator );
}


std::pair< DataRaw, DataRaw > split( const DataRaw & d )
{
    // Choose one subfolder.
    const auto labels = [ & d ] ()
        {
            std::vector<std::string> ret;
            for( const auto & kv : d )
            {
                ret.emplace_back( kv.first );
            }
            return ret;
        } ();
    const auto label = labels[ pick_index( labels.size() ) ];

    // Copy all instances of each label.
    DataRaw train, test;

    for( auto & kv : d )
    {
        const auto short_label = [ & kv ] ()
            {
                // Every label starts with '/'.
                const auto without_leading = kv.first.substr( 1 );
                const auto pos = without_leading.find_first_of( '/' );
                if( pos == std::string::npos )
                {
                    return kv.first;
                }
                else
                {
                    return kv.first.substr( 0, pos + 1 );
                }
            } ();

        const auto append = [ & kv ] ( std::vector<Spectrum> & list )
            {
                list.insert( list.end(), kv.second.cbegin(), kv.second.cend() );
            };

        if( kv.first == label )
        {
            append( test[ short_label ] );
        }
        else
        {
             append( train[ short_label ] );
        }
    }

    return { train, test };
}


Transcoder::Transcoder(const DataRaw &dat )
{
    for( const auto & kv : dat )
    {
        encode( kv.first );
    }
    assert( dat.size() == _encoding.size() );
    assert( _encoding.size() == _reverse.size() );
}


int Transcoder::encode( const std::string & l )
{
    if( _encoding.count( l ) == 0 )
    {
        _encoding[ l ] = static_cast<int>( _encoding.size() );
        _reverse[ static_cast<int>(  _encoding.size() ) ] = l ;
    }

    return _encoding[ l ];
}


int Transcoder::encode( const std::string & l ) const
{
    if( _encoding.count( l ) == 0 )
    {
        throw Exception{ "Label encoding failed. "
                         "Label '" + l + "' not found." };
    }

    return _encoding.at( l );
}


const std::string & Transcoder::decode( int i ) const
{
    if( _reverse.count( i ) == 0 )
    {
        throw Exception{ "Label decoding failed. "
                         "Value " + std::to_string( i ) + " not found." };
    }

    return _reverse.at( i );
}


Dataset encode( DataRaw & raw )
{
    Dataset dt{};

    for( auto & kv : raw )
    {
        dt.first.emplace( dt.second.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


Dataset encode( DataRaw & raw, const Transcoder & t )
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
void apply( std::function< void( int, const Spectrum & ) > f, const Dataset & d )
{
    for( const auto & label_vector : d.first )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void apply( std::function< void( const std::string &, const Spectrum & ) > f
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


}  // namespace dat
