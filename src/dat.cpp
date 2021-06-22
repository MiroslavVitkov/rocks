#include "dat.h"

#include <cassert>
#include <random>


namespace dat
{


void append( std::vector< Spectrum > & v1, const std::vector< Spectrum > & v2 )
{
    v1.insert( v1.end(), v2.cbegin(), v2.cend() );
}


std::pair< Dataset, Dataset > split( const Dataset & d
                                   , double traintest
                                   )
{
    assert( traintest > 0 && traintest < 1 );

    std::default_random_engine generator;
    generator.seed( 0 );
    std::uniform_real_distribution distribution( 0., 1. );

    Dataset train{ {}, d.second };
    Dataset test{ {}, d.second };

    for( const auto & kv : d.first )
    {
        const auto & label{ kv.first };
        for( const auto & datapoint : kv.second )
        {
            if( distribution( generator ) < traintest )
            {
                train.first[ label ].push_back( datapoint );
            }
            else
            {
                test.first[ label ].push_back( datapoint );
            }
        }
    }

    return { train, test };
}


Dataset encode( DataRaw && raw )
{
    Dataset dt;

    for( auto & kv : raw )
    {
        dt.first.emplace( dt.second.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


Dataset encode( DataRaw && raw, const label::Codec & t )
{
    Dataset dt{ {}, t };

    for( auto & kv : raw )
    {
        dt.first.emplace( t.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


Dataset encode( const DataRaw & raw, const label::Codec & t )
{
    Dataset dt{ {}, t };

    for( auto & kv : raw )
    {
        dt.first.emplace( t.encode( kv.first )
                        , kv.second );
    }

    return dt;
}


DataRaw decode( Dataset & d, const label::Codec & c )
{
    DataRaw raw{};

    for( auto & kv : d.first )
    {
        raw.insert_or_assign( c.decode( kv.first )
                            , std::move( kv.second )  );
    }

    return raw;
}


// The iteration order of unordered associative containers can only change
// when rehashing as a result of a mutating operation
// (as described in C++11 23.2.5/8).
void apply( std::function< void ( label::Num, const Spectrum & ) > f
          , const Dataset & d )
{
    for( const auto & label_vector : d.first )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void apply(std::function< void ( const label::Raw &, const Spectrum & ) > f
          , const DataRaw & d )
{
    for( const auto & label_vector : d )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void mutate( std::function< void ( const label::Raw &, Spectrum & ) > f
           , DataRaw & d )
{
    for( auto & label_vector : d )
    {
        for( auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void mutate( std::function< void ( label::Num, Spectrum & ) > f
           , Dataset & d )
{
    for( auto & label_vector : d.first )
    {
        for( auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


size_t count( const dat::Dataset & d )
{
    size_t total {};
    dat::apply( [ &total ] ( int, const dat::Spectrum & ) { ++total; }, d );
    return total;
}


size_t count( const dat::DataRaw & d )
{
    size_t total {};
    dat::apply( [ & total ] ( const std::string &, const dat::Spectrum & )
        { ++total; }, d );
    return total;
}


shark::RealVector to_shark_vector( const dat::Spectrum & s )
{
    return { s._y.cbegin(), s._y.cend() };
}


shark::ClassificationDataset to_shark_dataset( const dat::Dataset & d )
{
    if( d.first.empty() )
    {
        return {};
    }

    std::vector< shark::RealVector > inputs;
    std::vector< label::Num > labels;
    dat::apply( [&] ( label::Num l, const dat::Spectrum & s )
    {
        inputs.push_back( to_shark_vector( s ) );
        labels.push_back( static_cast< label::Num >( l ) );
    }
              , d );

    return shark::createLabeledDataFromRange( inputs, labels );
}


dat::Dataset from_shark_dataset( const shark::ClassificationDataset & d
                               , const label::Codec & c
                               )
{
    dat::Dataset ret{ {}, c };
    for( auto e{ d.elements().begin()}; e < d.elements().end(); ++e )
    {
        ret.first[ e->label ]; //.push_back( e->input );
        //vec.push_back( e->input );
    }
    return ret;
}


}  // namespace dat
