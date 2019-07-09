#include "dat.h"

#include "except.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>


namespace dat
{


void decompose_label_helper( const std::string & label
                           , size_t start
                           , std::vector< std::string > & ret )
{
    const auto pos = label.find_first_of( '/', start );

    if( pos != std::string::npos )
    {
        ret.emplace_back( label.substr( start, pos - start ) );
        decompose_label_helper( label, pos, ret );
    }
    else
    {
        return;
    }


}


// Decompose a hierarchical label.
std::vector< std::string > decompose_label( const std::string & label )
{
    std::vector< std::string > ret;
    decompose_label_helper( label, 0, ret );
    return ret;
}


std::string head( const std::string & full_label )
{
    assert( full_label[0] == '/' );
    const auto pos = full_label.find_first_of( '/', 1 );
    return full_label.substr( 0, pos );
}


void append( std::vector< Spectrum > v1, const std::vector< Spectrum > v2 )
{
    v1.insert( v1.end(), v2.cbegin(), v2.cend() );
}


std::pair< DataRaw, DataRaw > split( const DataRaw & d )
{
    DataRaw train, test;

    std::set< std::string > processed;
    for( const auto & kv : d )
    {
        const auto l = head( kv.first );

        if( processed.count( l ) )
        {
            append( train[ l ], kv.second );
        }
        else
        {
            append( test[ l ], kv.second );
            processed.emplace( l );
        }
    }

    return { train, test };


//    (void)d;
//    apply( [ & ] ( const std::string & label
//                 , const std::vector< Spectrum > & spectra )
//        {
//            const auto l = head( label );

//            if( processed.count( l ) )
//            {
//                train[ l ].emplace_back( spectra.cbegin(), spectra.cend() );
//            }
//            else
//            {
//                test[ l ].emplace_back( spectra.cbegin(), spectra.cend() );
//                processed.emplace( l );
//            }
//        }
//         , d );

//    std::function< void( const std::string &
//                             , const std::vector< Spectrum > & ) > f =
//    [ ] ( const std::string &
//                 , const std::vector< Spectrum > & )
//        {};
//    (void)f;
//    auto ff =
//    [ ] ( const std::string &
//                 , const std::vector< Spectrum > & )
//        { return;  };
//    (void) ff;

//    apply( f, d );

    // for each label
    // if first sublabel
    // then append all datapoints to test
    // else append all datapoints to train

#if 0
    for( auto & kv : d )
    {
        const auto short_label = [ & kv ] ()
            {
                // Every label starts with '/'.
                const auto without_leading = kv.first.substr( 1 );
                const auto pos = without_leading.find_first_of( '/' );
                if( pos == std::string::npos )
                {
                    return kv.first;
                }
                else
                {
                    return kv.first.substr( 0, pos + 1 );
                }
            } ();

        const auto append = [ & kv ] ( std::vector<Spectrum> & list )
            {
                list.insert( list.end(), kv.second.cbegin(), kv.second.cend() );
            };

        if( kv.first == label )
        {
            append( test[ short_label ] );
        }
        else
        {
             append( train[ short_label ] );
        }
    }
#endif
}


Transcoder::Transcoder(const DataRaw &dat )
{
    for( const auto & kv : dat )
    {
        encode( kv.first );
    }
    assert( dat.size() == _encoding.size() );
    assert( _encoding.size() == _reverse.size() );
}


int Transcoder::encode( const std::string & l )
{
    if( _encoding.count( l ) == 0 )
    {
        _encoding[ l ] = static_cast<int>( _encoding.size() );
        _reverse[ static_cast<int>(  _reverse.size() ) ] = l ;
    }

    return _encoding[ l ];
}


int Transcoder::encode( const std::string & l ) const
{
    if( _encoding.count( l ) == 0 )
    {
        throw Exception{ "Label encoding failed. "
                         "Label '" + l + "' not found." };
    }

    return _encoding.at( l );
}


const std::string & Transcoder::decode( int i ) const
{
    if( _reverse.count( i ) == 0 )
    {
        throw Exception{ "Label decoding failed. "
                         "Value " + std::to_string( i ) + " not found." };
    }

    return _reverse.at( i );
}

//labels printed should be only those present in the matrix
std::ostream & operator<<( std::ostream & s, const dat::Transcoder & t )
{
    for( const auto & kv : t._reverse )
    {
        s << kv.first << ": " << kv.second << ", ";
    }

    return s;
}


Dataset encode( DataRaw & raw )
{
    Dataset dt{};

    for( auto & kv : raw )
    {
        dt.first.emplace( dt.second.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


Dataset encode( DataRaw & raw, const Transcoder & t )
{
    Dataset dt{ {}, t };

    for( auto & kv : raw )
    {
        dt.first.emplace( t.encode( kv.first )
                        , std::move( kv.second )  );
    }

    return dt;
}


// The iteration order of unordered associative containers can only change
// when rehashing as a result of a mutating operation
// (as described in C++11 23.2.5/8).
void apply( std::function< void( int, const Spectrum & ) > f, const Dataset & d )
{
    for( const auto & label_vector : d.first )
    {
        for( const auto & s : label_vector.second )
        {
            f( label_vector.first, s );
        }
    }
}


void apply( std::function< void( const std::string &, const Spectrum & ) > f
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


}  // namespace dat
