#include "score.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/matrix.h>
#endif

#include <algorithm>
#include <cassert>
#include <random>
#include <set>


namespace score
{


#ifdef CMAKE_USE_DLIB
const dat::Spectrum & find_worst( Comp c, const dat::Dataset & d )
{
    using P = std::pair< int, const dat::Spectrum & >;
    std::vector< P > scores;
    dat::apply( [ & ] ( int, const dat::Spectrum & s )
        {
            scores.emplace_back( c( s ), s );
        }
             , d );

    const auto & m = * std::max_element( scores.cbegin()
                                       , scores.cend()
                                       , [] ( const P & p1, const P & p2 )
        {
            int a = p1.first;
            int b = p2.first;
            return a > b;
            //return p1.first < p2.second;
        }    );

    return m.second;
}


std::set< label::Num > get_classes( const std::vector< label::Num > & v1
                                  , const std::vector< label::Num > & v2 )
{
    std::set< label::Num > all{ v1.cbegin(), v1.cend() };
    all.insert( v2.cbegin(), v2.cend() );
    return all;
}


struct Transcoder
{
    Transcoder( const std::set< label::Num > & classes )
    {
        int counter {};
        for( const auto c : classes )
        {
            _label_to_encoded[ c ] = counter++;
        }
    }

    int encode( label::Num label )
    {
        return _label_to_encoded[ label ];
    }

private:
    std::unordered_map<int, int> _label_to_encoded;
};


// Challange: some predicted labels may not exist in the ground_truth.
// Challange: labels can be arbitrary numbers.
dlib::matrix< unsigned >
calc_confusion( const std::vector< label::Num > &ground_truth
              , const std::vector< label::Num > &predicted )
{
    assert( ground_truth.size() == predicted.size() );

    const auto classes = get_classes( ground_truth, predicted );
    Transcoder t{ classes };
    const long c = static_cast<long>( classes.size() );
    dlib::matrix<unsigned> ret = dlib::zeros_matrix<unsigned>( c, c );

    for( unsigned i {}; i < ground_truth.size(); ++i )
    {
        const auto row = t.encode( ground_truth[ i ] );
        const auto col = t.encode( predicted[ i ] );
        ++ret( row, col );
    }

    return ret;
}


double accuracy( const Confusion & c )
{
    const auto t{ dlib::trace( c ) };
    const auto s{ dlib::sum( c ) };
    return 1.0 * t / s;
}
#endif  // CMAKE_USE_DLIB


}  // namespace score
