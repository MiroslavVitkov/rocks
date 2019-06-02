#ifndef IO_H_
#define IO_H_


#include "except.h"

#include <array>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>


namespace io
{


// https://physics.nist.gov/cgi-bin/ASD/lines1.pl?compositionCu&spectra=Cu&low_w=180&limits_type=0&upp_w=961&show_av=2&unit=1&resolution=1&temp=1&eden=1e17&libs=1
// https://www.nist.gov/pml/atomic-spectroscopy-compendium-basic-ideas-notation-data-and-formulas/atomic-spectroscopy
// https://www.nist.gov/pml/atomic-spectra-database
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


using DatasetRaw = std::unordered_map<std::string, std::vector<Spectrum>>;
using DatasetEncoded = std::unordered_map<int, std::vector<Spectrum>>;


// Example of the expected file structure.
//data
//├── azurite
//│   ├── spot00
//│   │   ├── 10.csv
//│   │   ├── 11.csv
//│   │   ├── 12.csv
//│   │   ├── 14.csv
//│   │   ├── 15.csv
//│   │   ├── 16.csv
//│   │   ├── 19.csv
//│   │   └── 1.csv
//│   └── spot01
//│       └── 24.csv
//└── brochantite
//    ├── 99.csv
//    └── spot00
//        └── 7.csv
// Any non-directories or non .csv files are ignored.
DatasetRaw read_dataset( const std::string & path );


struct Transcoder
{
    Transcoder() = default;
    Transcoder( const DatasetRaw & );
    Transcoder( std::vector<std::string>, std::vector<std::string> );

    int encode( const std::string & l );
    int encode( const std::string & l ) const;
    const std::string & decode( int i ) const;

private:
    std::unordered_map<std::string, int> _encoding;
    std::unordered_map<int, std::string> _reverse;
};


std::tuple<DatasetEncoded, Transcoder> encode_dataset( DatasetRaw & );


// Legacy.
using Label = std::string;
using Dataset = std::unordered_map<Label, std::vector<Spectrum>>;


}  // namespace io


#endif // defined(IO_H_)
