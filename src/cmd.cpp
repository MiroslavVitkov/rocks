#include "cmd.h"

#include "io.h"

#include <iostream>


namespace cmd
{


Train::Train(const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void Train::execute()
{
    const auto dataset = io::read_dataset( _data_dir );
    std::cout << "kopele tva e golqmo" << size(dataset);
}



}  // namespace cmd
