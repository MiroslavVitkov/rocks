#include "io.h"

#include <cassert>
#include <filesystem>
#include <fstream>


namespace fs = std::filesystem;


namespace io
{


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
    assert( line == "wavelength,intensity" );

    Spectrum ret;
    auto it = ret._y.begin();

    while( file )
    {
        double wavelength, intensity;
        char comma;
        file >> wavelength >> comma >> intensity;
        *it++ = intensity;
    }
    assert( it == ret._y.end() );

    return ret;
}


Dataset read_dataset( const std::string & path )
{
    // All top-level dirs found in 'path' are label names.
    std::vector<std::string> labels;
    for( const auto & dir : fs::directory_iterator( path ) )
    {
        if( fs::is_directory( dir ) )
        {
            labels.push_back( dir.path().filename() );
        }
    }

    // All .csv files under a label are samples of that label.
    Dataset ret;
    for( const auto & l : labels )
    {
        for( const auto & file : fs::recursive_directory_iterator( path + l ) )
        {
            if( ends_with( file.path().filename(), ".csv" ) )
            {
                ret[ l ].emplace_back( read_csv( file ) );
            }
        }
    }

    return ret;
}

/*

std::vector<std::tuple<std::string, std::vector<Spectrum>>>
read_dataset( const std::string & path)
{

}



Spectrum read_file( const std::string & path );
void read_dir( std::vector<Spectrum> & spectra
             , std::string & label
             , const std::string path );
std::vector<std::string> get_subdirs( const std::string & path );

Spectrum read_file( const std::string & path );


void read_dir( std::vector<Spectrum> & spectra
             , std::string & label
             , const std::string path );


// True if entry is the unix current of parent directory.
bool is_auto_dir( const struct dirent * const ent )
{
    bool is_currdir = ! std::string( ent->d_name ).compare( "." );
    bool is_pardir  = ! std::string( ent->d_name ).compare( ".." );
    return is_currdir || is_pardir;
}


std::vector<std::string> get_subdirs( const std::string & path )
{
    DIR * dir = opendir( path.c_str() );
    if( ! dir )
    {
        throw Exception{ "Failed to open dir: " + path };
    }

    std::vector<std::string> out;
    while( struct dirent * stream = readdir( dir ) )
    {
        assert( stream );

        if( is_auto_dir( stream ) )
        {
            continue;
        }

        std::string temp{ &stream->d_name };
        out.insert( temp );
    }
    closedir( dir );

    return out;
}







std::vector<std::tuple<std::string, std::vector<Spectrum>>>
 read_dataset( const std::string & path);
*/


}  // namespace io
