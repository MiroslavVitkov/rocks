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


unsigned find_labels_depth( const Parser & p )
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
}


auto find_preprocessing( const Parser & p )
{
    std::vector< std::string > ret;
    if( p.option( "p" ) )  // TODO: allow multiple -p!
    {
        const auto algo = p.option( "p" ).argument();
        ret.push_back( algo );
    }

    return ret;
}


Cmd create_model( const Parser & p )
{
    assert( p.option( "m" ).count() );
    const auto model_name{ p.option( "m" ).argument() };

    // Verify such a model exists by creating one with an empy training set.
    model::create( model_name, {} );

    return std::make_unique< cmd::RunModel >( find_dataset( p )
                                            , model_name
                                            , find_labels_depth( p )
                                            , find_preprocessing( p )
                                            );
}


void show_models()
{
    std::string all{"Models: "};
    for( const auto & m : model::ALL_MODELS )
    {
        all += m + ", ";
    }

    print::info( all );
}


void show_preprocessing()
{

    std::string all{ "Data preprocessing algos: " };
    for( const auto & p : pre::ALL_PRE )
    {
        all += p + ", ";
    }
    print::info( all );
}


Cmd parse( int argc, Argv argv )
{
    Parser p;
    p.add_option( "h", "Print this." );
    p.add_option( "help", "Print this." );

    p.add_option( "o", "Produce a report on outliers." );
    p.add_option( "m", "Execute <model>.", 1 );
    p.add_option( "l", "How many <levels> of subdirs to capture into hierarchic labels.", 1 );
    p.add_option( "d", "Path to dataset root dir.", 1 );
    p.add_option( "s", "Show all available models and preprocessing algorithms." );
    p.add_option( "a", "Run all models. Obviously very slow." );
    p.add_option( "p", "Use <algorithm> to preprocess the dataset.", 1 );

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

    if( p.option( "s" ) )
    {
        show_models();
        show_preprocessing();
        return std::make_unique< cmd::NoOp >();
    }

    if( p.option( "a" ) )
    {
        return std::make_unique< cmd::RunAllModels >( find_dataset( p )
                                                    , find_labels_depth( p )
                                                    );
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
