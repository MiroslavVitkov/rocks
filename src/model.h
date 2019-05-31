#ifndef MODEL_H_
#define MODEL_H_


#include "io.h"

#include <filesystem>
#include <map>


namespace model
{


//using io::Label = io::Label;  // and confidence  // or prob dist?


struct Model
{
    virtual io::Label predict( const io::Spectrum & ) const = 0;
    //virtual void serialise( const std::filesystem::path & ) const = 0;
    //static std::unique_ptr<Model> deserialise( const std::filesystem::path & ) const;
};


struct RandomChance : Model
{
    RandomChance( const io::Dataset & );
    io::Label predict( const io::Spectrum & ) const override;

private:
    std::map<io::Label, double> _probs;
};


struct Correlation : Model
{
    Correlation( const io::Dataset & );
    io::Label predict( const io::Spectrum & ) const override;

private:
    const io::Dataset & _training_set;
};


}  // namespace model


#endif  // #ifndef MODEL_H_
