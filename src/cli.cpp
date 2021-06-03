#include "cli.h"
#include "model.h"
#include "print.h"

#ifdef CMAKE_USE_DLIB
#include <dlib/cmd_line_parser.h>
#endif

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>


namespace cli
{



std::unique_ptr<cmd::Base> parse( int argc, Argv argv )
{
#ifdef CMAKE_USE_DLIB
    dlib::command_line_parser parser;
    parser.add_option( "h", "Print this message." );
    parser.add_option( "help", "Print this message." );
    parser.add_option( "m", "Execute model of <name> or list avaible models."
                     , 1 );
    parser.add_option( "model", "Execute model of <name> or list avaible models."
                     , 1 );
    parser.add_option( "o", "Print a report on outliers" );
    parser.add_option( "outliers", "Print a report on outliers" );
    parser.add_option( "d", "How deep to descend in subdirectories of the dataset."
                     , 1 );
    parser.add_option( "labels-depth", "How deep to descend in subdirectories of the dataset."
                     , 1 );

    parser.parse( argc, const_cast< char** >( argv ) );

    if( parser.option( "h" ) || parser.option( "help" ) )
    {
        parser.print_options();
        return std::make_unique<cmd::NoOp>();
    }

    if( parser.option( "m" ) || parser.option( "model" ) )
    {
        // If only -m is passed list models and exit.
        if( ! parser.option( "m" ).count() && ! parser.option( "model" ).count() )
        {
            for( const auto & s : model::ALL_MODELS )
            {
                print::info( s );
            }
            return std::make_unique< cmd::NoOp >();
        }

        const auto model_name =[ & parser ] ()
        {
            if( parser.option( "m" ) )
            {
                return parser.option( "m" ).argument();
            }
            else
            {
                return parser.option( "model" ).argument();
            }

        } ();


        const unsigned labels_depth =[ & parser ] ()
        {
            if( parser.option( "d" ) )
            {
                const auto wtf = parser.option( "d" ).argument();
                return std::stoi( wtf );
            }
            else if( parser.option( "labels-depth" ) )
            {
                return std::stoi( parser.option( "labels-depth" ).argument() );
            }
            else
            {
                return 1;
            }
        } ();


        // Verify such a model exists by creating one with an empy training set.
        auto m = model::create( model_name, {} );

        return std::make_unique< cmd::RunModel >( "../rocks/data"
                                                , model_name
                                                , labels_depth
                                                );
    }

    if( parser.option( "o" ) || parser.option( "outliers" ) )
    {
        return std::make_unique<cmd::ReportOutliers>( "../rocks/data" );
    }
#endif  // CMAKE_USE_DLIB
    (void)argc;
    (void)argv;

    std::cerr << "No action selected. Exiting.\n";
    return std::make_unique<cmd::NoOp>();
}



}  // namespace cli
