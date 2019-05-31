#include "cmd.h"

#include "io.h"
#include "model.h"

#include <iostream>


namespace cmd
{


RandomChance::RandomChance(const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void RandomChance::execute()
{
    const auto dataset = io::read_dataset( _data_dir );
    // split into train and test
    model::RandomChance m{ dataset };
    std::string s{"azurite"};
    const auto & az = dataset.at(s);
    const io::Spectrum & first = az[0];
    std::cout << m.predict( first );
}


Correlation::Correlation(const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void Correlation::execute()
{
    const auto dataset = io::read_dataset( _data_dir );
    // split into train and test
    model::Correlation m{ dataset };
    std::string s{"azurite"};
    const auto & az = dataset.at(s);
    const io::Spectrum & first = az[0];
    std::cout << m.predict( first );
}



}  // namespace cmd
