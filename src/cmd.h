#ifndef CMD_H_
#define CMD_H_


#include <memory>
#include <string>
#include <vector>


namespace cmd
{


struct Base
{
    virtual void execute() = 0;
    virtual ~Base() = default;
};


struct NoOp : Base
{
    void execute() override {}
};


// https://physics.nist.gov/cgi-bin/ASD/lines1.pl?compositionCu&spectra=Cu&low_w=180&limits_type=0&upp_w=961&show_av=2&unit=1&resolution=1&temp=1&eden=1e17&libs=1
// abscissa - from 180.0nm to 960.9nm inclusive with 0.1nm step.
// ordinate - radiance, W·sr−1·m−2
// number of points - 7810
struct Spectrum
{

};

struct Corpus
{
    std::vector<Spectrum> _spectra;
    const std::string _label;
};

// Example of the file structure that we are expecting.
//data
//├── azurite
//│   ├── spot00
//│   ├── spot01
//│   ├── spot10
//│   ├── spot11
//├── brochantite
//│   ├── spot0
//│   ├── spot1
//│   ├── spot2
struct Train : Base
{
    Train( const std::string & data_dir);
    void execute() override {}
};


}  // namespace cmd


#endif  // defined(CMD_H_)
