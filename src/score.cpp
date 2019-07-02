#include "score.h"

#include <evaluation.hpp>

#include <algorithm>
#include <cassert>
#include <random>
#include <set>


namespace score
{


void evaluate_and_print( const std::vector<int> & targets
                       , const std::vector<int> & outputs)
{
    Confusion c( targets, outputs );
    Evaluation e( c );

    c.print();
    e.print();
}


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

}  // namespace score
