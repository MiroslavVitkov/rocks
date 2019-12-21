#ifndef DAT_H_
#define DAT_H_


// In this file:
//     operations over the dataset:
//     1. the datapoint type throughout the project,
//     2. splitting into train and test detasets
//     3. walking the dataset sequentially.


#include "label.h"

#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>


namespace dat
{


// A single spectroscope measurement for a single point.
// In some representation.
template < typename ElemT, unsigned num_dims >
struct Sample
{
    using value_type = ElemT;
    static constexpr auto _num_points{ num_dims };
    using Axis = std::array< ElemT, num_dims >;

    Axis _y {};
};


// Each .csv file contains one Spectrum.
// Each spectrum contains 7810 intensity values from 180nm to 960.9nm.
// _x: wavelength, nm
// _y: radiance, W·sr−1·m−2
struct Spectrum : Sample< double, 7810 >
{
    static constexpr Axis _x{ [] ()
        {
            Axis a {};
            for( unsigned i = 0; i < _num_points; ++i )
            {
                a[ i ] = 180 + 0.1 * i;
            }
            return a;
        } () };
};


struct Compressed : Sample< float, 5 >
{
};


using DataRaw = std::unordered_map< label::Raw, std::vector< Spectrum > >;
using DataEncoded = std::unordered_map< label::Num, std::vector< Spectrum > >;
using Dataset = std::pair< DataEncoded, label::Codec >;


// For each label, pick the first subdir as test, the rest as train.
// Requires 2 levels of hierarchical labels.
// Trims the labels to only the major part.
std::pair< DataRaw, DataRaw > split( const DataRaw & );

Dataset encode( DataRaw & );
Dataset encode( DataRaw &, const label::Codec & );

// Invoke provided functor on every element in a dataset.
// Walking order is consistent until the dataset is altered.
void apply( std::function< void ( label::Num, const Spectrum & ) >
          , const Dataset & );
void apply( std::function< void ( const label::Raw &, const Spectrum & ) >
          , const DataRaw & );

void mutate ( std::function< void ( const label::Raw &, Spectrum & ) >
            , DataRaw & );

// Count total number of spectra.
size_t count( const dat::Dataset & );
size_t count( const dat::DataRaw & );


}  // namespace dataset


#endif // defined( DAT_H_ )
