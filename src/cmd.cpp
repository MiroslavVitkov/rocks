#include "cmd.h"

#include "io.h"
#include "model.h"
#include "score.h"

#include <iostream>
#include <vector>


namespace cmd
{


RandomChance::RandomChance(const std::string & data_dir )
    : _data_dir{ data_dir }
{}


void RandomChance::execute()
{
    // Obtain the dataset.
    auto raw = io::read_dataset( _data_dir );
    const auto encoded = io::encode_dataset( raw );
    const auto traintest = score::train_test_split( encoded );

    // Train the model.
    model::RandomChance model{ traintest.first };

    // Evaluate the training set.
    std::vector<int> targets, outputs;
    for( const auto & lebel_vector : traintest.second.first )
    {
        for( const auto & tespoint : lebel_vector.second )
        {
            targets.push_back( lebel_vector.first );
            outputs.push_back( model.predict( tespoint ) );
        }
    }

    // Report.
    std::cout << "Reporting statistics on a RandomChance model.\n";
    score::evaluate_and_print( targets, outputs );
}


Correlation::Correlation(const std::string & data_dir )
    : _data_dir{ data_dir }
{
}


void Correlation::execute()
{
    // Obtain the dataset.
    auto raw = io::read_dataset( _data_dir );
    const auto encoded = io::encode_dataset( raw );
    const auto traintest = score::train_test_split( encoded );

    // Train the model.
    model::Correlation model{ traintest.first };

    // Evaluate the training set.
    std::vector<int> targets, outputs;
    for( const auto & lebel_vector : traintest.second.first )
    {
        for( const auto & tespoint : lebel_vector.second )
        {
            targets.push_back( lebel_vector.first );
            outputs.push_back( model.predict( tespoint ) );
        }
    }

    // Report.
    std::cout << "Reporting statistics on a Correlation model.\n";
    score::evaluate_and_print( targets, outputs );
}



}  // namespace cmd
