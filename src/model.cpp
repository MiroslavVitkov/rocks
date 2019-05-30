#include "model.h"


namespace model
{


RandomChance::RandomChance( const io::Dataset & )
{

}


Prediction RandomChance::predict( const io::Value & ) const
{
    return "big dick nigga";
}


}  // namespace model
