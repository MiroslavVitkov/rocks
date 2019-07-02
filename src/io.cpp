#include "io.h"

#include "except.h"

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


// "213.00000000,14.0001\r"  ->  14.0001
double parse( const std::string & line )
{
    const size_t comma = line.find_first_of( ',' );
    const auto sub = line.substr( comma + 1 );
    const auto d = std::stod( sub );

    return d;
}


dat::Spectrum read_csv( const fs::path & path )
{
    std::ifstream file{ path };
    std::string line;

    // Drop the column names.
    std::getline( file, line);
    assert( line == "wavelength,intensity\r" );

    dat::Spectrum ret;
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


// By default top-level dirs found in 'path' are label names.
// All .csv files under a label are samples of that label.
dat::DataRaw read( const std::string & dataset_dir , unsigned labels_depth )
{
    dat::DataRaw ret;
    for( const auto & file : fs::recursive_directory_iterator( dataset_dir ) )
    {
        if( ends_with( file.path().filename(), ".csv" ) )
        {
            const auto full_label = fs::relative( file.path(), dataset_dir );
            std::string label;
            try
            {
                for( auto it = full_label.begin()
                   ; std::distance( full_label.begin(), it ) < labels_depth
                   ; ++it )
                {
                    label += '/';
                    label += *it;
                }
            }
            catch( ... )
            {
                throw Exception{ "Requested labels_depth = "
                               + std::to_string( labels_depth )
                               + ", but file "
                               + full_label.string()
                               + " is located at smaller depth." };
            }

            ret[ label ].emplace_back( read_csv( file ) );
        }
    }

    return ret;
}


}  // namespace io
