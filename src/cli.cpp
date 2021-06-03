#include "cli.h"
#include "print.h"

#ifdef CMAKE_USE_DLIB

#include "model.h"

#include <dlib/cmd_line_parser.h>

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>


namespace cli
{


using Cmd = std::unique_ptr< cmd::Base >;
using Parser = dlib::command_line_parser;


std::filesystem::path find_dataset( const Parser & p )
{
    if( p.option( "d" ) )
    {
        return p.option( "d" ).argument();
    }
    else
    {
        return "../rocks/data";
    }
}


Cmd create_model( const Parser & p )
{
    assert( p.option( "m" ).count() );

    // If only -m is passed list models and exit.
    if( ! p.option( "m" ).count())
    {
        for( const auto & s : model::ALL_MODELS )
        {
            print::info( s );
        }
        return std::make_unique< cmd::NoOp >();
    }

    const auto model_name{ p.option( "m" ).argument() };

    const unsigned labels_depth = [ & p ] ()
    {
        if( p.option( "l" ) )
        {
            const auto labels_depth = p.option( "l" ).argument();
            return std::stoi( labels_depth );
        }
        else
        {
            return 1;
        }
    } ();

    // Verify such a model exists by creating one with an empy training set.
    const auto m = model::create( model_name, {} );

    return std::make_unique< cmd::RunModel >( find_dataset( p )
                                            , model_name
                                            , labels_depth
                                            );
}


Cmd parse( int argc, Argv argv )
{
    Parser p;
    p.add_option( "h", "Print this message." );
    p.add_option( "help", "Print this message." );

    p.add_option( "o", "Produce a report on outliers." );
    p.add_option( "m", "Execute <model> or list avaible models.", 1 );
    p.add_option( "l", "How many <levels> of subdirs to capture into hierarchic labels.", 1 );
    p.add_option( "d", "Path to dataset root dir.", 1 );

    p.parse( argc, const_cast< char** >( argv ) );

    if( p.option( "h" ) || p.option( "help" ) )
    {
        p.print_options();
        return std::make_unique< cmd::NoOp >();
    }

    if( p.option( "o" ) )
    {
        return std::make_unique< cmd::ReportOutliers >( find_dataset( p ) );
    }

    if( p.option( "m" ) )
    {
        return create_model( p );
    }

    print::info( "No action selected. Exiting.\n" );
    return std::make_unique< cmd::NoOp >();
}


#else  // CMAKE_USE_DLIB


std::unique_ptr<cmd::Base> parse( int, Argv )
{
    print::info( "dlib is required for command line parsing. Exiting." );
    return std::make_unique< cmd::NoOp >();
}


#endif  // CMAKE_USE_DLIB


}  // namespace cli
