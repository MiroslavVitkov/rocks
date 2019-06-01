#ifndef IO_H_
#define IO_H_


#include "except.h"

#include <array>
#include <string>
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

    static constexpr Axis _x{ [] () constexpr  // wavelength, nm
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


using Label = std::string;
using Dataset = std::unordered_map<Label, std::vector<Spectrum>>;


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
Dataset read_dataset( const std::string & path );


struct Transcoder
{
    int encode( const io::Label & l );
    const io::Label & decode( int i );

private:
    std::unordered_map<io::Label, int> _encoded;
    std::unordered_map<int, io::Label> _reverse;
};



}  // namespace io


#endif // defined(IO_H_)
