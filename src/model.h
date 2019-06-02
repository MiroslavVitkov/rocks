#ifndef MODEL_H_
#define MODEL_H_


#include "io.h"

#include <filesystem>
#include <unordered_map>


namespace model
{


struct Model
{
    virtual int predict( const io::Spectrum & ) const = 0;
    //virtual void serialise( const std::filesystem::path & ) const = 0;
    //static std::unique_ptr<Model> deserialise( const std::filesystem::path & ) const;
    virtual ~Model() = default;
};


struct RandomChance : Model
{
    RandomChance( const io::Dataset & );
    int predict( const io::Spectrum & ) const override;

private:
    std::unordered_map<int, double> _probs;
};


struct Correlation : Model
{
    Correlation( const io::Dataset & );
    int predict( const io::Spectrum & ) const override;

private:
    const io::Dataset & _training_set;
};


}  // namespace model


#endif  // #ifndef MODEL_H_
