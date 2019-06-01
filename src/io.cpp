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


Dataset read_dataset( const std::string & path )
{
    // All top-level dirs found in 'path' are label names.
    const auto labels{ get_subdirs( path ) };

    // All .csv files under a label are samples of that label.
    Dataset ret;
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


int Transcoder::encode( const io::Label & l )
{
    if( _encoded.count( l ) == 0 )
    {
        _encoded[ l ] = static_cast<int>( _encoded.size() );
        _reverse[ static_cast<int>(  _encoded.size() ) ] = l ;
    }

    return _encoded[ l ];
}


const io::Label & Transcoder::decode( int i )
{
    if( _reverse.count( i ) == 0 )
    {
        throw Exception{ "Label decoding failed. "
                         "Value " + std::to_string( i ) + " not found." };
    }

    return _reverse[ i ];
}


}  // namespace io
