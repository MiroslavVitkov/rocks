#include "cli.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>


namespace cli
{


std::unique_ptr<cmd::Base> parse( int argc, Argv argv )
{
    (void)argc;
    (void)argv;
    // TODO: #include <dlib/cmd_line_parser.h>
    // http://dlib.net/dlib/cmd_line_parser/cmd_line_parser_kernel_abstract.h.html
    return std::make_unique<cmd::Correlation>( "../rocks/data" );
}


}  // namespace cli
