#include "io.h"

#include <cassert>
#include <filesystem>
#include <fstream>


namespace io
{


namespace fs = std::filesystem;


// Waiting for C++20's std::basic_string::ends_with().
inline bool ends_with(std::string const & value, std::string const & ending)
{
    if( ending.size() > value.size() )
    {
        return false;
    }
    return std::equal( ending.rbegin(), ending.rend(), value.rbegin() );
}


double parse( const std::string & line )
{
    const size_t comma = line.find_first_of( ',' );
    const auto sub = line.substr( comma + 1 );
    const auto d = std::stod( sub );

    return d;
}


Spectrum read_csv( const fs::path & path )
{
    std::ifstream file{ path };
    std::string line;

    // Drop the column names.
    std::getline( file, line);
    assert( line == "wavelength,intensity\r" );

    Spectrum ret;
    auto it = ret._y.begin();

    while( std::getline( file, line) )
    {
        double d = parse( line );
        *it++ = d;
    }

    // Make sure we read the correct number of datapoints.
    assert( it == ret._y.end() );

    return ret;
}


std::vector< std::string > get_subdirs( const std::string & path )
{
    std::vector< std::string > ret;
    for( const auto & dir : fs::directory_iterator( path ) )
    {
        if( fs::is_directory( dir ) )
        {
            ret.push_back( dir.path().filename() );
        }
    }

    return ret;
}


DatasetRaw read_dataset( const std::string & path )
{
    // All top-level dirs found in 'path' are label names.
    const auto labels{ get_subdirs( path ) };

    // All .csv files under a label are samples of that label.
    DatasetRaw ret;
    for( const auto & l : labels )
    {
        for( const auto & file
           : fs::recursive_directory_iterator( path + '/' + l ) )
        {
            if( ends_with( file.path().filename(), ".csv" ) )
            {
                ret[ l ].emplace_back( read_csv( file ) );
            }
        }
    }

    return ret;
}


Transcoder::Transcoder( const DatasetRaw & dat )
{
    for( const auto & kv : dat )
    {
        encode( kv.first );
    }
    assert( dat.size() == _encoding.size() == _reverse.size() );
}


Transcoder::Transcoder( std::vector<std::string> a, std::vector<std::string> b )
{
    for( const auto & e : a )
    {
        encode( e );
    }
    for( const auto & e : b )
    {
        encode( e );
    }

    assert( ! a.empty() );
    assert( ! _encoding.empty() );
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
                         "Label " + l + " not found." };
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


std::tuple<DatasetEncoded, Transcoder> encode_dataset( DatasetRaw & raw )
{
    std::tuple<DatasetEncoded, Transcoder> ret;
    auto & dataset{ std::get<0>( ret ) };
    auto & codec{ std::get<1>( ret ) };

    for( auto & kv : raw )
    {
        dataset.emplace( codec.encode( kv.first )
                       , std::move( kv.second )  );
    }
    return ret;
}


}  // namespace io
