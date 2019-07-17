#include "pre.h"

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

}  // namespace pre
