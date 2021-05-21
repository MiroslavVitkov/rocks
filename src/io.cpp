#include "io.h"

#include "except.h"
#include "print.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <ranges>
#include <vector>


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
    if( line != "wavelength,intensity\r" )
    {
        throw Exception{ path.string() + " is wrong format." };
    }

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


// Basically lists subdirectories.
std::vector< std::string > extract_sorted_labels( const fs::path & base_dir )
{
    std::vector< fs::path > labels;
    for( const auto & dir_label : fs::directory_iterator( base_dir ) )
    {
        if( ! dir_label.is_directory() )
        {
            const auto & name = dir_label.path().string();
            print::info( std::string( "Skipping non-directory in dataset: ") + name );
            continue;
        }
        labels.push_back( dir_label );
    }

    std::sort( labels.begin(), labels.end() );

    std::vector< std::string > ret{};
    for(const auto & l : labels)
    {
        ret.emplace_back( l.filename().string() );
    }

    return ret;
}


std::vector< fs::path > recursively_list_csvs( const std::string & dir )
{
    std::vector< fs::path > files;
    for( const auto & file : fs::recursive_directory_iterator( dir ) )
    {
        if( ends_with( file.path().filename(), ".csv" ) )
        {
            files.emplace_back( file.path() );
        }
    }
    return files;
}


std::vector< dat::Spectrum > recursively_read_csvs( const std::string & path )
{
    const auto files{ recursively_list_csvs( path ) };

    std::vector< dat::Spectrum > ret;
    for( const auto & file : files)
    {
        try
        {
            ret.push_back( read_csv( file ) );
        }
        catch( ... )
        {
        }
    }

    // How do we make this exception-tolerant?
//    std::transform( files.cbegin()
//                  , files.cend()
//                  , std::back_inserter( ret )
//                  , read_csv
//                  );
    return ret;
}


// By default top-level dirs found in 'path' are label names.
// All .csv files under a label are samples of that label.
// 'dataset_dir' remains contant through the recursion.
dat::DataRaw read( const fs::path & dataset_dir
                 , unsigned labels_depth
                 , const std::string & labels_prefix
                 )
{
    dat::DataRaw ret{};

    if( labels_depth > 0 )
    {
        const auto subpath{ [&] () { return dataset_dir.string() + SEPARATOR + labels_prefix; } () };
        const auto & labels{ extract_sorted_labels( subpath ) };
        for( const auto & l : labels )
        {
            const auto label{ labels_prefix + SEPARATOR + l };
            const auto part = read( dataset_dir, labels_depth - 1 , label );
            assert( ret[ l ].empty() );
            for( auto p = part.begin(); p != part.end(); ++p )
            {

            }
        }
    }
    else
    {
        ret[ labels_prefix ] = recursively_read_csvs( dataset_dir );
    }

    return ret;


    ///////////////

    const auto & labels{ extract_sorted_labels( dataset_dir ) };
    for( const auto & l : labels )
    {
        //const auto label = labels_prefix + dataset_dir + l;
        auto label = fs::path( labels_prefix );
        label /= l;
        // TODO: multiple issues here
        //  - why can't part be const?
        //  - portability
        const auto part = read( dataset_dir, labels_depth - 1 , label );
        assert( ret[ l ].empty() );
        for( auto p = part.begin(); p != part.end(); ++p )
        {
//<<<<<<< HEAD
//            const auto full_label = fs::relative( file.path(), dataset_dir );
//            std::string label;
//            try
//            {
//                for( auto it = full_label.begin()
//                   ; std::distance( full_label.begin(), it ) < labels_depth
//                   ; ++it )
//                {
//                    label += '/';
//                    label += *it;
//                }
//            }
//            catch( ... )
//            {
//                throw Exception{ "Requested labels_depth = "
//                               + std::to_string( labels_depth )
//                               + ", but file "
//                               + full_label.string()
//                               + " is located at smaller depth." };
//            }

//            try
//            {
//                ret[ label ].push_back( read_csv( file ) );
//            }
//            catch( const Exception & e )
//            {
//                print::info( e.what() );
//            }
//=======
            print::info( p->first );
//>>>>>>> feeaf90 (a mess! nearly ready hyeararchic labels)
        }
        print::info( "KUR" );
//        assert( sizeof(part) == sizeof(part[l]) );
//        ret[l] = part[l];
    }

//    const auto label = fs::relative( dir, dataset_dir );

    return ret;
}


}  // namespace io
