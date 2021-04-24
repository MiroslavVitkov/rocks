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


void logarithm( dat::DataRaw & d )
{
    double min {};
    dat::apply( [ & ] ( const std::string &, const dat::Spectrum & s )
        {
            for( const auto & point : s._y )
            {
                if( point < min )
                {
                    min = point;
                }
            }
        }      , d );
    dat::mutate( [ & ] ( const std::string &, dat::Spectrum & s )
        {
            for( auto & point : s._y )
            {
                const auto positive = point - min + 1;
                point = std::log( positive );
            }
        }      , d );
}


void normalize( dat::DataRaw & d )
{
    dat::Spectrum mean {};
    dat::apply( [ & ] ( const std::string &, const dat::Spectrum & s )
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
    dat::apply( [ & ] ( const std::string &, const dat::Spectrum & s )
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

    dat::mutate( [ & ] ( const std::string &, dat::Spectrum & s )
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


}  // namespace pre
