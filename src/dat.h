#ifndef DAT_H_
#define DAT_H_


// In this file:
//     operations over the dataset:
//     1. the datapoint type throughout the project,
//     2. splitting into train and test detasets
//     3. walking the dataset sequentially.


#include "label.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/matrix.h>
#endif

#ifdef CMAKE_USE_SHARK
#include <shark/Data/Dataset.h>
#endif

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
    using pointer = ElemT *;
    using const_pointer = ElemT const *;
    static constexpr auto _num_points{ num_dims };
    using Axis = std::array< ElemT, num_dims >;

    value_type operator[]( size_t i ) { return _y[i]; };

    pointer begin(){ return _y; };
    const_pointer cbegin(){ return & _y; };
    pointer end(){ return _y + _num_points; };
    const_pointer cend(){ return & _y; };

    virtual ~Sample() = default;

    // The actual data.
    Axis _y {};
};


// Each .csv file contains one Spectrum.
// Each spectrum contains 7810 intensity values from 180nm to 960.9nm.
// _x: wavelength, nm
// _y: radiance, W·sr−1·m−2
struct Spectrum : Sample< double, 7810 >
{
    static constexpr Axis _x =
    []
    ()
    {
        Axis a {};
        for( unsigned i = 0; i < _num_points; ++i )
        {
            a[ i ] = 180 + 0.1 * i;
        }
        return a;
    }
    ();
};


// Dimensionality reduction target.
struct Compressed : Sample< float, 100 >
{
};


using DataRaw = std::unordered_map< label::Raw, std::vector< Spectrum > >;
using DataEncoded = std::unordered_map< label::Num, std::vector< Spectrum > >;
using Dataset = std::pair< DataEncoded, label::Codec >;

// Dimensionally reduced dataset.
using DataCompressed = std::unordered_map< label::Num, std::vector< Compressed > >;
using DatasetCompressed = std::pair< DataCompressed, label::Codec >;


// Perform holdout split.
// Sample points at random without regard to label. TODO: stratified sampler.
// `traintest` ranges from 0 - only test to 1 - only train.
std::pair< Dataset, Dataset > split( const Dataset &
                                   , double traintest=0.66 );
std::pair< DatasetCompressed, DatasetCompressed > split( const DatasetCompressed &
                                                       , double traintest=0.66 );


Dataset encode( DataRaw && );
Dataset encode( DataRaw &&, const label::Codec & );
Dataset encode( const DataRaw & raw, const label::Codec & );

DataRaw decode( Dataset &&, const label::Codec & );

// Invoke provided functor on every element in a dataset.
// Walking order is consistent until the dataset is altered.
void apply( std::function< void ( label::Num, const Spectrum & ) >
          , const Dataset & );

void mutate( std::function< void ( label::Num, Spectrum & ) >
           , Dataset & );

// Count total number of spectra.
size_t count( const Dataset & );

#ifdef CMAKE_USE_SHARK
shark::RealVector to_shark_vector( const Spectrum & );
shark::ClassificationDataset to_shark_dataset( const Dataset & );
shark::ClassificationDataset to_shark_dataset( const DataRaw &
                                             , const label::Codec &
                                             );

// Inefficient due to quadratic complexity of traversing a shark dataset.
Spectrum from_shark_vector( const shark::RealVector & );
Dataset from_shark_dataset( const shark::ClassificationDataset &
                          , const label::Codec &
                          );
#endif  // CMAKE_USE_SHARK

#ifdef CMAKE_USE_DLIB
// Dlib also features flexible dynamic sizing.
// But we are using the more type-safe and efficient static version.
using DlibSample = dlib::matrix< Spectrum::value_type, dat::Spectrum::_num_points, 1 >;
using DlibFlattened = std::pair< std::vector< DlibSample >
                               , std::vector< label::Num > >;

DlibSample to_dlib_sample( const Spectrum & );
#endif  // CMAKE_USE_DLIB

}  // namespace dataset


#endif // defined( DAT_H_ )
