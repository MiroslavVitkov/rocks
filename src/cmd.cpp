#include "cmd.h"

#include "io.h"
#include "model.h"
#include "score.h"

#include <iostream>
#include <vector>


namespace cmd
{


RunModel::RunModel( const std::string & data_dir
                  , const std::string & model_name )
    : _data_dir{ data_dir }
    , _model_name{ model_name }
{
}


void RunModel::execute()
{
    // Obtain the dataset.
    auto raw = io::read_dataset( _data_dir );
    const auto encoded = io::encode_dataset( raw );
    const auto traintest = score::train_test_split( encoded );

    // Train the model.
    const auto m = model::create( _model_name, traintest.first );

    // Evaluate the test set.
    std::vector<int> targets, outputs;
    io::walk( traintest.second
            , [ & ] ( int label, const io::Spectrum & s )
        {
            const std::vector<io::Spectrum> v{ s };

            targets.push_back( label );
            outputs.push_back(  m->predict( v ) );
        } );

    // Report.
    score::evaluate_and_print( targets, outputs );
}


}  // namespace cmd
