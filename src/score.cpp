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


std::set<int> get_classes( const std::vector<int> & v1
                            , const std::vector<int> & v2 )
{
    std::set<int> all{ v1.cbegin(), v1.cend() };
    all.insert( v2.cbegin(), v2.cend() );
    return all;
}


struct Transcoder
{
    Transcoder( const std::set<int> & classes )
    {
        int counter {};
        for( const auto c : classes )
        {
            _label_to_encoded[ c ] = counter++;
        }
    }

    int encode( int label )
    {
        return _label_to_encoded[ label ];
    }

private:
    std::unordered_map<int, int> _label_to_encoded;
};


// Challange: some predicted labels may not exist in the ground_truth.
// Challange: labels can be arbitrary numbers.
dlib::matrix< unsigned > calc_confusion( const std::vector<int> & ground_truth
                                       , const std::vector<int> & predicted )
{
    assert( ground_truth.size() == predicted.size() );

    const auto classes = get_classes( ground_truth, predicted );
    Transcoder t{ classes };
    dlib::matrix< unsigned > ret( static_cast<long>( classes.size() )
                                , static_cast<long>( classes.size() ) );

    for( unsigned i {}; i < ground_truth.size(); ++i )
    {
        const auto row = t.encode( ground_truth[ i ] );
        const auto col = t.encode( predicted[ i ] );
        ++ret( row, col );
    }

    return ret;
}


}  // namespace score
