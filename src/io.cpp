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


Spectrum read_csv( const fs::path & path )
{
    std::ifstream file{ path };
    std::string line;

    // Drop the column names.
    std::getline( file, line);
    assert( line == "wavelength,intensity\r" );

    Spectrum ret;
    auto it = ret._y.begin();

    double wavelength, intensity;
    char comma;
    while( file >> wavelength >> comma >> intensity )
    {
        *it++ = intensity;
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


}  // namespace io
