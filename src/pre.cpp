#include "pre.h"

#include "label.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/statistics.h>
#include <dlib/svm.h>
#endif

#include <cassert>
#include <cmath>


namespace pre
{


dat::Dataset logarithm( const dat::Dataset & d )
{
    double min {};
    dat::apply( [ & ] ( label::Num, const dat::Spectrum & s )
        {
            for( const auto & point : s._y )
            {
                if( point < min )
                {
                    min = point;
                }
            }
        }     , d );

    dat::Dataset ret{ {}, d.second };
    dat::apply( [ & ] ( label::Num l, const dat::Spectrum & s )
        {
            dat::Spectrum transformed{};
            unsigned i {};
            for( const auto & intensity : s._y )
            {
                const auto positive = intensity - min + 1;
                const auto point = std::log( positive );
                transformed._y[ i++ ] = point;
            }
            ret.first[ l ].push_back( transformed );
        }      , ret );

    return ret;
}


Log::Log( const dat::Dataset & )
{
}


dat::Dataset Log::operator()( const dat::Dataset & d ) const
{
    return logarithm( d );
}


void normalize( dat::Dataset & d )
{
    dat::Spectrum mean {};
    dat::apply( [ & ] ( label::Num, const dat::Spectrum & s )
    {
        auto src = s._y.cbegin();
        const auto end = s._y.cend();
        auto dest = mean._y.begin();
        while( src < end )
        {
            * dest++ += * src++;
        }
    }      , d );
    const auto c = dat::count( d );
    for( auto & point : mean._y )
    {
        point /= c;
    }

    dat::Spectrum variance;
    dat::apply( [ & ] ( label::Num, const dat::Spectrum & s )
    {
        auto sp = s._y.cbegin();
        const auto end = s._y.cend();
        auto dest = variance._y.begin();
        while( sp < end )
        {
            const auto a = mean._y[ sp - s._y.cbegin()] ;
            const auto diff = * sp - a;
            * dest += diff * diff;
            assert( diff * diff >= 0 );
            ++ sp;
            ++ dest;
        }
    }      , d );
    for( auto & point : variance._y )
    {
        point = sqrt( point / ( c - 1 ) );
        assert( point >= 0 );
    }

    dat::mutate( [ & ] ( label::Num, dat::Spectrum & s )
    {
        auto src = s._y.begin();
        const auto end = s._y.cend();
        auto m_it = mean._y.cbegin();
        auto v_it = variance._y.cbegin();
        while( src < end )
        {
            if( * v_it < 1e-12  )
            {
                * src = * m_it;
            }
            else
            {
                * src = ( * src - * m_it ) / * v_it;
            }

            ++src;
            ++m_it;
            ++v_it;
        }
    }      , d );
}



Norm::Norm( const dat::Dataset & )
{
}


dat::Dataset Norm::operator()( const dat::Dataset & d ) const
{
    dat::Dataset ret{ d };
    normalize( ret );
    return ret;
}



std::vector< size_t > rank_features( const dat::Dataset & )
{
    return {};
}


#ifdef CMAKE_USE_DLIB
dat::Dataset lda( const dat::Dataset & d )
{
    dlib::matrix< double > samples;
    samples.set_size( static_cast< long >( dat::count( d ) )
                    , dat::Spectrum::_num_points );
    std::vector< unsigned long > labels;
    auto i = 0u;
    dat::apply( [ & ] ( const label::Num l, const dat::Spectrum & s )
        {
            labels.push_back( static_cast< unsigned long >( l ) );
            std::copy( s._y.cbegin()
                     , s._y.cend()
                     , samples.begin() + dat::Spectrum::_num_points * i++ );
        }     , d );
    const auto samples2 = samples;

    dlib::matrix< double, 0, 1 > means;
    dlib::compute_lda_transform( samples, means, labels );

    dlib::matrix< double > ret = samples2 * samples - means; //crash
    std::cout << samples.nc() << ", " << samples.nr() << std::endl;
    std::cout << samples2.nc() << ", " << samples2.nr() << std::endl;
    std::cout << ret.nc() << ", " << ret.nr() << std::endl;

    return {};
}
#endif // CMAKE_USE_DLIB


shark::LinearModel<>
train_encoder( std::vector< shark::RealVector > & inputs
             , unsigned N
             )
{
    const auto dataset{ shark::createUnlabeledDataFromRange( inputs ) };
    shark::PCA pca{ dataset };
    shark::LinearModel<> enc;
    pca.encoder( enc, N );
    return enc;
}


shark::LinearModel<> train_encoder( const dat::Dataset & train
                                  , unsigned N
                                  )
{
    std::vector< shark::RealVector > inputs;
    dat::apply( [ & ] ( auto, const dat::Spectrum & s )
    {
        inputs.push_back( dat::to_shark_vector( s ) );
    }
    , train
    );

    return train_encoder( inputs, N );
}

shark::LinearModel<>
train_encoder( const shark::ClassificationDataset & train
             , unsigned N
             )
{
    std::vector< shark::RealVector > vec;
    for( auto e{ train.elements().begin()}; e < train.elements().end(); ++e )
    {
        vec.push_back( e->input );
    }

    return train_encoder( vec, N );
}


PCA::PCA( const dat::Dataset & train, unsigned dim )
    : _enc{ train_encoder( train, dim ) }
{
}


PCA::PCA( const shark::ClassificationDataset & train, unsigned dim )
    : _enc{ train_encoder( train, dim ) }
{
}


dat::Dataset PCA::operator()( const dat::Dataset & d ) const
{
    if( d.first.empty() )
    {
        return {};
    }

    std::vector< shark::RealVector > inputs;
    std::vector< label::Num > labels;
    dat::apply( [&] ( label::Num l, const dat::Spectrum & s )
    {
        shark::RealVector enc;
        const auto vec{ dat::to_shark_vector( s ) };
        _enc.eval( vec, enc );
        inputs.push_back( std::move( enc ) );
        labels.push_back( l );
    }
              , d );

    const auto sharkd{ shark::createLabeledDataFromRange( inputs, labels ) };
    return dat::from_shark_dataset( sharkd, d.second );
}


Simple::Simple( const dat::Dataset & )
{
}


dat::Compressed Simple::operator()( const dat::Spectrum & s ) const
{
    const auto mean = std::accumulate( s._y.begin(), s._y.end(), 0 ) / s._num_points;
    using CompressedValue = dat::Compressed::value_type;
    const auto squared_diff = [ mean ] ( CompressedValue sum, CompressedValue v )
        { return sum + ((v - mean) * (v - mean)); };
    const auto variance = std::accumulate( s._y.begin(), s._y.end(), 0, squared_diff ) / s._num_points;

    dat::Compressed ret{};
    ret._y[0] = mean;
    ret._y[1] = variance;
    return ret;
}


const std::vector< std::string > ALL_PRE{ "log"
                                        , "norm"

#if defined(CMAKE_USE_OPENCV) || defined(CMAKE_USE_SHARK)
                                         , "pca"
#endif
#if defined (CMAKE_USE_OPENCV) || defined (CMAKE_USE_DLIB)
                                        , "lda"
#endif
                                        };


}  // namespace pre
