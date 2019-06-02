#ifndef IO_H_
#define IO_H_


// In this file:
//     1. fundamental data types, used throughout the project,
//     2. reading the dataset from disk.
//
// Example of the expected file structure.
// data                     <- arbitrary name
// ├── azurite              <- subdirs are labels
// │   ├── spot00           <- any subsubdirs are flattened out
// │   │   ├── 10.csv
// │   │   ├── 11.csv
// │   │   ├── 12.csv
// │   │   ├── 14.csv
// │   │   ├── 15.csv
// │   │   ├── 16.csv
// │   │   ├── 19.csv
// │   │   └── 1.csv
// │   └── spot01
// │       └── 24.csv
// └── brochantite
//     ├── 99.csv
//     └── spot00
//         └── 7.csv
// Any non-directories or non .csv files are ignored.


#include "except.h"

#include <array>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace io
{


// Each .csv file contains one Spectrum.
struct Spectrum
{
    static constexpr unsigned _num_points{ 7810 };
    using Axis = std::array< double, _num_points >;

    static constexpr Axis _x{ [] ()            // wavelength, nm
        {
            Axis a {};
            for( unsigned i = 0; i < _num_points; ++i )
            {
                a[ i ] = 180 + 0.1 * i;
            }
            return a;
        } () };
    Axis _y {};                                // radiance, W·sr−1·m−2
};


using DataRaw = std::unordered_map< std::string, std::vector< Spectrum > >;
using DataEncoded = std::unordered_map< int, std::vector< Spectrum > >;


DataRaw read_dataset( const std::string & path );


struct Transcoder
{
    Transcoder() = default;
    Transcoder( const DataRaw & );

    int encode( const std::string & l );
    int encode( const std::string & l ) const;
    const std::string & decode( int i ) const;

private:
    std::unordered_map<std::string, int> _encoding;
    std::unordered_map<int, std::string> _reverse;
};


using Dataset = std::pair< DataEncoded, Transcoder >;


Dataset encode_dataset( DataRaw & );


}  // namespace io


#endif // defined(IO_H_)
