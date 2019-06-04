#include "cli.h"
#include "model.h"

#include <dlib/cmd_line_parser.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>


namespace cli
{


std::unique_ptr<cmd::Base> parse( int argc, Argv argv )
{
    dlib::command_line_parser parser;
    parser.add_option( "h", "Print this message." );
    parser.add_option( "help", "Print this message." );
    parser.add_option( "model", "See 'rocks/src/model.h' for the complete list."
                     , 1 );
    parser.parse( argc, const_cast< char** >( argv ) );

    if( parser.option( "h" ) || parser.option( "help" ) )
    {
        parser.print_options();
        return std::make_unique<cmd::NoOp>();
    }

    if( ! parser.option( "model" ) )
    {
        std::cerr << "No model selected. Exiting.\n";
        return std::make_unique<cmd::NoOp>();
    }

    const auto model_name = parser.option( "model" ).argument();

    // Verify such a model exists by creating one with empy training set.
    auto m = model::create( model_name, {} );

    return std::make_unique<cmd::RunModel>( "../rocks/data"
                                          , model_name );
}


}  // namespace cli
