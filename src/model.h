#ifndef MODEL_H_
#define MODEL_H_


#include "io.h"

#include <filesystem>


namespace model
{


using Prediction = io::Key;  // and confidence


struct Model
{
    virtual Prediction predict( const io::Value & ) const = 0;
    //virtual void serialise( const std::filesystem::path & ) const = 0;
    //static std::unique_ptr<Model> deserialise( const std::filesystem::path & ) const;
};


struct RandomChance : Model
{
    RandomChance( const io::Dataset & );
    Prediction predict( const io::Value & ) const override;
};


}  // namespace model


#endif  // #ifndef MODEL_H_
