#include "dim.h"

#include "dat.h"

#ifdef CMAKE_USE_OPENCV
#include <opencv2/core.hpp>
#endif

#include <algorithm>
#include <numeric>
#include <tuple>


namespace dim
{


dat::DatasetCompressed Base::operator()( const dat::Dataset & d ) const
{
    dat::DatasetCompressed ret{ {}, d.second };
    dat::apply(
    [ & ret, this ]
    ( label::Num l, const dat::Spectrum & s )
    {
        ret.first[ l ].push_back( ( * this )( s ) );
    }
    , d );

    return ret;
}


#ifdef CMAKE_USE_OPENCV
#if 0

cv::LDA init_lda( const dat::Dataset & d )
{
    if( ! dat::count( d ) )
    {
         return cv::LDA{};
    }

    cv::Mat dataset( static_cast< int >( dat::count( d ) )
                   , dat::Spectrum::_num_points
                   , CV_64FC1 );
    int i {};
    std::vector< label::Num > labels;
    dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
    {
        std::copy( s._y.cbegin()
                 , s._y.cend()
                 , dataset.row( i++ ).begin< double >() );
        labels.push_back( l );
    }, d );
    assert( labels.size() == static_cast< size_t >( dataset.rows ) );

    cv::LDA lda( dataset
               , labels
               , static_cast< int >( dat::Compressed::_num_points )
               );
    return lda;
}


LDA::LDA( const dat::Dataset & d )
    : _lda{ init_lda( d ) }
{
}


dat::Compressed LDA::operator()( const dat::Spectrum & s ) const
{
    auto ss{ s };
    void * p = ( ss._y.data() );
    const int cols = dat::Spectrum::_num_points;
    const cv::Mat m( 1, cols, CV_64FC1, p );

    const auto proj = _lda.project( m );

    dat::Compressed ret;
    assert( static_cast< size_t >( proj.cols ) == ret._y.size() );
    assert( proj.isContinuous() );
    ret._y = proj;
    return ret;
}
#endif

cv::PCA init_pca( const dat::Dataset & d )
{
    if( ! dat::count( d ) )
    {
        return {};
    }

    cv::Mat dataset( static_cast< int >( dat::count( d ) )
                   , dat::Spectrum::_num_points
                   , CV_64FC1 );
    int i {};
    dat::apply( [ & ] ( label::Num, const dat::Spectrum & s )
    {
        std::copy( s._y.cbegin()
                 , s._y.cend()
                 , dataset.row( i++ ).begin< double >() );
    }, d );

    cv::PCA pca( dataset, cv::Mat{}, cv::PCA::DATA_AS_ROW
               , static_cast< int >( dat::SpectrumCompressed::_num_points ) );
    return pca;
}


PCA::PCA( const dat::Dataset & d )
    : _pca{ init_pca( d ) }
{
}


dat::SpectrumCompressed PCA::operator()( const dat::Spectrum & s ) const
{
    cv::Mat m( 1, dat::Spectrum::_num_points, CV_64FC1 );
    std::copy( s._y.cbegin(), s._y.cend(), m.begin< double >() );

    const auto projected = _pca.project( m );

    dat::SpectrumCompressed ret;
    assert( projected.rows == 1 );
    for( unsigned i {}; i < dat::SpectrumCompressed::_num_points; ++i )
    {
        // We are converting from double to dat::Compressed::value_type.
        const auto val = projected.at< double >( static_cast< int >( i ) );
        ret._y[i] = static_cast< float >( val );
    }

    return ret;
}


#endif  // CMAKE_USE_OPENCV


Simple::Simple( const dat::Dataset & )
{
}


dat::SpectrumCompressed Simple::operator()( const dat::Spectrum & s ) const
{
    const auto mean = std::accumulate( s._y.begin(), s._y.end(), 0 ) / s._num_points;
    using CompressedValue = dat::SpectrumCompressed::value_type;
    const auto squared_diff = [ mean ] ( CompressedValue sum, CompressedValue v )
        { return sum + ((v - mean) * (v - mean)); };
    const auto variance = std::accumulate( s._y.begin(), s._y.end(), 0, squared_diff ) / s._num_points;

    dat::SpectrumCompressed ret{};
    ret._y[0] = mean;
    ret._y[1] = variance;
    return ret;
}


const std::vector< std::string > ALL{ "simple"
                                    };

}  // namespace dim
