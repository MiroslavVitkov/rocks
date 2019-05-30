#include "cmd.h"

#include "io.h"
#include "model.h"

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
    model::RandomChance m{ io::Dataset{} };
    std::cout << m.predict( io::Value{} ) << size(dataset);
}



}  // namespace cmd
