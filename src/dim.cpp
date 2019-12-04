#include "dim.h"

#include "dat.h"

#include <dlib/matrix.h>
#include <dlib/statistics.h>

#include <opencv2/core.hpp>

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
            X( row, col++ ) = static_cast< float >( a );
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


cv::PCA init_pca( const dat::Dataset & d )
{
    if( ! dat::count( d ) )
    {
        return {};
    }

    cv::Mat dataset( static_cast< int >( dat::count( d ) )
                   , dat::Spectrum::_num_points
                   , CV_64FC1 );
    int row {};
    dat::apply( [ & ] ( label::Num, const dat::Spectrum & s )
    {
        std::copy( s._y.cbegin()
                 , s._y.cend()
                 , dataset.row( row++ ).begin< double >() );
    }, d );

    cv::PCA pca( dataset, cv::Mat{}, cv::PCA::DATA_AS_ROW
               , static_cast< int >( dat::Compressed::_num_points ) );
    return pca;
}


PCA::PCA( const dat::Dataset & d )
    : _pca{ init_pca( d ) }
{
}


dat::Compressed PCA::operator()( const dat::Spectrum & s ) const
{
    cv::Mat m( 1, dat::Spectrum::_num_points, CV_64FC1 );
    std::copy( s._y.cbegin(), s._y.cend(), m.data );

    const auto projected = _pca.project( m );

    dat::Compressed ret;
    projected.copyTo( ret._y );
    return ret;
}


}  // namespace dim
