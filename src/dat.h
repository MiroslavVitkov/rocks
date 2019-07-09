#ifndef DAT_H_
#define DAT_H_


// In this file:
//     operations over the dataset:
//     1. the datapoint type throughout the project,
//     2. transcoding the labels between strings and ints,
//     3. splitting into train and test detasets.


#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <iostream>
namespace dat
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


struct Transcoder
{
    Transcoder() = default;
    Transcoder( const DataRaw & );

    int encode( const std::string & l );
    int encode( const std::string & l ) const;
    const std::string & decode( int i ) const;

    friend std::ostream & operator<<( std::ostream &, const dat::Transcoder & );
private:
    std::unordered_map<std::string, int> _encoding;
    std::unordered_map<int, std::string> _reverse;
};
std::ostream & operator<<( std::ostream &, const dat::Transcoder& );


using DataEncoded = std::unordered_map< int, std::vector< Spectrum > >;
using Dataset = std::pair< DataEncoded, Transcoder >;


// For each label, pick the first subdir as test, the rest as train.
// Requires 2 levels of hierarchical labels.
std::pair< DataRaw, DataRaw > split( const DataRaw & );

Dataset encode( DataRaw & );
Dataset encode( DataRaw &, const Transcoder & );

// Invoke provided functor on every element in a dataset.
// Walking order is consistent until the dataset is altered.
void apply( std::function< void( int, const Spectrum & ) >
          , const Dataset & );
void apply( std::function< void( const std::string &, const Spectrum & ) >
          , const DataRaw & );


}  // namespace dataset


#endif // defined( DAT_H_ )
