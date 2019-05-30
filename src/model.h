#ifndef MODEL_H_
#define MODEL_H_


#include "io.h"

#include <filesystem>
#include <map>


namespace model
{


using Prediction = io::Key;  // and confidence  // or prob dist?


struct Model
{
    virtual Prediction predict( const io::Spectrum & ) const = 0;
    //virtual void serialise( const std::filesystem::path & ) const = 0;
    //static std::unique_ptr<Model> deserialise( const std::filesystem::path & ) const;
};


struct RandomChance : Model
{
    RandomChance( const io::Dataset & );
    Prediction predict( const io::Spectrum & ) const override;

private:
    std::map<io::Key, double> _probs;
};


}  // namespace model


#endif  // #ifndef MODEL_H_
