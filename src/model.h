#ifndef MODEL_H_
#define MODEL_H_


// In this file:
//               1. models predicting the class of stone,
//               2. factory from std::string (at the end).


#include "dat.h"
#include "except.h"

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>


namespace model
{


struct Model
{
    // Assumption: all test spectra are drawn from the same population.
    // Assumption: all test samples in the vector belong to the same class.
    virtual label::Num predict( const dat::Spectrum & ) const = 0;

    //virtual void serialise( const std::filesystem::path & ) const = 0;
    //static std::unique_ptr<Model> deserialise( const std::filesystem::path & ) const;

    virtual ~Model() = default;
};


struct RandomChance : Model
{
    RandomChance( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;

private:
    std::unordered_map<int, double> _probs;
};


struct Correlation : Model
{
    Correlation( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;

private:
    const dat::Dataset & _training_set;
    const std::vector< label::Num > _labels;
};


struct SVM : Model
{
    SVM( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;

private:
    using Sample = dlib::matrix< double, dat::Spectrum::_num_points, 1 >;
    using Flattened = std::pair< std::vector< Sample >
                               , std::vector< label::Num > >;
    const Flattened _training_set;
};


inline std::unique_ptr<Model> create( const std::string & name
                                    , const dat::Dataset & d )
{
    const auto is = [ & name ] ( const char * p )
        { return ( name.compare( p ) == 0 ); };

    if( is( "chance" ) )
    {
        return std::make_unique< RandomChance >( RandomChance( d ) );
    }
    if( is( "correlation" ) )
    {
        return std::make_unique< Correlation >( Correlation( d ) );
    }
    if( is( "svm" ) )
    {
        return std::make_unique< SVM >( SVM( d ) );
    }

    throw Exception( name + ": no such model found."
                     "See 'model.h' for a list of all models." );
}


}  // namespace model


#endif  // #ifndef MODEL_H_
