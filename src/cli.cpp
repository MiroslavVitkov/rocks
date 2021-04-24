#include "cli.h"
#include "model.h"

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
    parser.add_option( "m", "See 'rocks/src/model.h' for the complete list."
                     , 1 );
    parser.add_option( "model", "See 'rocks/src/model.h' for the complete list."
                     , 1 );
    parser.add_option( "o", "Print a report on outliers" );
    parser.add_option( "outliers", "Print a report on outliers" );

    parser.parse( argc, const_cast< char** >( argv ) );

    if( parser.option( "h" ) || parser.option( "help" ) )
    {
        parser.print_options();
        return std::make_unique<cmd::NoOp>();
    }

    if( parser.option( "m" ) || parser.option( "model" ) )
    {
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

        // Verify such a model exists by creating one with an empy training set.
        auto m = model::create( model_name, {} );

        return std::make_unique< cmd::RunModel >( "../rocks/data"
                                                , model_name );
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
