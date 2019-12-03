#include "dim.h"

#include "dat.h"

#include <dlib/matrix.h>
#include <dlib/statistics.h>

#include <algorithm>
#include <tuple>


namespace dim
{


std::pair< dlib::matrix< float >
         , std::vector< unsigned long > > dataset_to_mat( const dat::Dataset & d )
{
    const auto nrows = static_cast< long >( dat::count( d ) );
    const auto ncols = dat::Spectrum::_num_points;
    dlib::matrix< float > X( nrows , ncols );

    unsigned row {};
    std::vector< unsigned long > labels;
    dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
    {
        // Too bad std::copy() doesn't work with dlib::matrix.
        unsigned col {};
        for( const auto a: s._y )
        {
            X( row, col++ ) = a;
        }

        labels.push_back( static_cast< unsigned long >( l ) );
        ++row;
        col = 0;

    }, d );

    return { X, labels };
}



LDA::LDA( const dat::Dataset & d )
{
    if( d.first.empty() )
    {
        return;
    }

    const auto X = dataset_to_mat( d );
    _Z = X.first;
    const auto row_labels = X.second;
    assert( row_labels.size() == static_cast< size_t >( X.first.nr() ) );

    // LDA (at least the implementation in sklearn) can produce at most
    // k-1 components (where k is number of classes).
    dlib::compute_lda_transform( _Z, _M, row_labels, dat::Compressed::_num_points );
    assert( _Z.nr() == dat::Compressed::_num_points
         && _Z.nc() == dat::Spectrum::_num_points );
}


dat::Compressed LDA::operator()( const dat::Spectrum & s ) const
{
    // No ranged initialization available?!
    dlib::matrix< T, 1, dat::Spectrum::_num_points > sample;
    auto writer = sample.begin();
    for( const auto & a : s._y )
    {
        *writer++ = a;
    }

    dlib::matrix< T > ret1 =_Z * sample;
    dlib::matrix< T > ret2 = ret1 - _M;

    dat::Compressed ret;
    assert( static_cast< size_t  >( ret2.size() ) == ret._y.size() );
    std::copy( ret2.begin(), ret2.end(), ret._y.begin() );
    return ret;
}


PCA::PCA( const dat::Dataset & )
{

}


dat::Compressed PCA::operator()( const dat::Spectrum & ) const
{
    dlib::discriminant_pca< dlib::matrix< double > > dpca;
    const auto m = dpca.dpca_matrix_of_size( dat::Compressed::_num_points );
    return {};
}


/*
struct LDA
{
    using T = double;


    LDA( const dlib::matrix< T > & X
       , const std::vector< unsigned long > & row_labels )
        : _Z{ X }
    {
        assert( row_labels.size() == X.nr() );
        dlib::compute_lda_transform( _Z, _M, row_labels );
    }


    dlib::matrix< T > foreward( dlib::matrix< T > x )
    {
        assert( _Z.size() != 0 );
        assert( x.nr() == 1 );
        assert( _Z.nc() == x.nr() );
        dlib::matrix<T> ret1 =_Z * x;
        dlib::matrix<T> ret2 = ret1 - _M;
        return ret2;
    }


private:
    dlib::matrix< T > _Z;
    dlib::matrix< T, 0, 1 > _M;
};
*/

dlib::matrix< double >
//std::vector<std::vector<double>>
foo(std::vector<dat::Spectrum> &)
{
    using T = double;
    dlib::matrix< T > X;  // Each row of X is one input vector. Output Z in it.
    dlib::matrix< T, 0, 1 > M;  // Output.
    std::vector< unsigned long > row_labels;

    // Z*x-M maps x into a space where x vectors that share the same class label
    // are near each other.
    dlib::compute_lda_transform( X, M, row_labels, 1, 0 );

    return {};
}


#include <dlib/statistics.h>
#include <iostream>

using namespace std;
using namespace dlib;

int main2(){
    typedef matrix<double,2,1> Sample;

    std::vector<unsigned long> labels;
    std::vector<Sample> samples;

    for (int i=0; i<4; i++){
        Sample s;
        s(0) = i;
        s(1) = i+1;
        samples.push_back(s);
        labels.push_back(1);

        Sample s1;
        s1(0) = i+1;
        s1(1) = i;
        samples.push_back(s1);
        labels.push_back(2);
    }

    matrix<double> X;
    X.set_size(8,2);
    for (unsigned i=0; i<8; i++){
        X(i,0) = samples[i](0);
        X(i,1) = samples[i](1);
    }

    matrix<double,0,1> mean;

    dlib::compute_lda_transform(X,mean,labels,1,0);

    for (unsigned i=0; i<samples.size(); i++){
        cout << X*samples[i]-mean << endl;
    }

    getchar();
    return 0;
}

}  // namespace dim
