#include "pre.h"

#include "label.h"

#include <dlib/statistics.h>
#include <dlib/svm.h>

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


std::vector< size_t > rank_features( const dat::Dataset & d )
{
    // Most naive approach - look at only the first two classes.
    using Sample = dlib::matrix< double, dat::Spectrum::_num_points, 1 >;
    using Kernel = dlib::radial_basis_kernel< Sample >;

    std::vector< Sample > samples;
    std::vector< label::Num > labels;
    dat::apply( [ & ] ( const label::Num l, const dat::Spectrum & s )
        {
            labels.push_back( l );
            Sample sample;
            std::copy( s._y.cbegin(), s._y.cend(), sample.begin() );
            samples.push_back( sample );
        }     , d );

    // Normalize the data.
    const Sample m(dlib::mean(dlib::mat(samples)));  // compute a mean vector
    const Sample sd(dlib::reciprocal(dlib::stddev(dlib::mat(samples)))); // compute a standard deviation vector
    for (unsigned long i = 0; i < samples.size(); ++i)
    {
        samples[i] = pointwise_multiply(samples[i] - m, sd);
        for( const auto ss : samples[i] )
        {
        if( std::isnan( ss ) )
        {
            throw 7;
        }
        }
    }
    dlib::randomize_samples(samples,labels);

    const auto gamma = dlib::verbose_find_gamma_with_big_centroid_gap( samples
                                                                     , labels );
    dlib::kcentroid< Kernel > kc( Kernel( gamma ), 0.001, 25 );
     const auto rank = dlib::rank_features( kc, samples, labels );

    std::vector< size_t > ret;
    for( auto i = 0u; i < dat::Spectrum::_num_points; ++i )
    {
        ret.push_back( static_cast< size_t >( rank( i, 0 ) ) );
    }


    return ret;
}


}  // namespace pre
