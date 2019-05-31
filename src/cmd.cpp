#include "cmd.h"

#include "io.h"
#include "model.h"
#include "score.h"

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
#if 0
    const auto dataset = io::read_dataset( _data_dir );
    const auto t = score::train_test_split( dataset );
    const auto train = std::get<0>(t);
    const auto test = std::get<1>(t);
    std::cout << "train = " << train.size() << "; test = " << test.size();
#endif

    const auto dataset = io::read_dataset( _data_dir );
    const auto t = score::train_test_split( dataset );
    const auto train = std::get<0>( t );
    const auto test = std::get<1>( t );

    model::Correlation m{ train };

    for( const auto & kv : test )
    {
        for( const auto & datapoint : kv.second )
        {
            std::cout << "Predicted: " << m.predict( datapoint )
                      << ", actual: " << kv.first << '\n';
        }
    }
}



}  // namespace cmd
