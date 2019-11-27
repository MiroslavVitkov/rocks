#ifndef DIM_H_
#define DIM_H_


// In this file: dimensionality reduction techniques.


#include "dat.h"

#include <dlib/matrix.h>

#include <array>
#include <vector>


namespace dim
{


using value_type = double;
constexpr unsigned num_dims = 5;
using Compressed = std::array< value_type, num_dims >;


// Linear Discriminant Analysis
// Z*x-M maps x into a space where x vectors that share the same class label
// are near each other.
struct LDA
{
    using T = value_type;

    LDA( const dat::Dataset & );
    Compressed operator()( const dat::Spectrum & ) const;

private:
    dlib::matrix< T > _Z;
    dlib::matrix< T, 0, 1 > _M;
};


}  // namespace dim


#endif  // #ifndef DIM_H_
