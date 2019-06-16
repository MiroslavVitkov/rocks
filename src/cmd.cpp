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
    auto raw = io::read( _data_dir );
    const auto encoded = io::encode( raw );
    const auto traintest = score::train_test_split( encoded );

    // Train the model.
    const auto m = model::create( _model_name, traintest.first );

    // Evaluate the test set.
    std::vector<int> targets;
    std::vector<io::Spectrum> flattened;
    io::apply( [ & ] ( int l, const io::Spectrum & s )
        {
            targets.push_back( l );
            flattened.push_back( s );
        }
             , traintest.second );
    const std::vector outputs( flattened.size(), m->predict( flattened ) );

    // Report.
    score::evaluate_and_print( targets, outputs );
}


}  // namespace cmd
