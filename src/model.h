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


#ifdef CMAKE_USE_DLIB
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
    ~SVM() override;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};


struct LDAandSVM : Model
{
    LDAandSVM( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;
    ~LDAandSVM() override;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};


struct PCAandSVM : Model
{
    PCAandSVM( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;
    ~PCAandSVM() override;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};


struct NN : Model
{
    NN( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;
    ~NN() override;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};
#endif  // CMAKE_USE_DLIB


// Use Shark or andres's implementation.
struct Forest : Model
{
    Forest( const dat::Dataset & );
    label::Num predict( const dat::Spectrum & ) const override;
    ~Forest() override;

private:
    struct Impl;
    std::unique_ptr< Impl > _impl;
};


inline std::unique_ptr< Model > create( const std::string & name
                                      , const dat::Dataset & d )
{
    const auto is = [ & name ] ( const char * p )
        { return ( name.compare( p ) == 0 ); };

    if( is( "chance" ) )
    {
        return std::make_unique< RandomChance >( RandomChance( d ) );
    }
#ifdef CMAKE_USE_DLIB
    if( is( "cor" ) )
    {
        return std::make_unique< Correlation >( Correlation( d ) );
    }
    if( is( "svm" ) )
    {
        // std::make_unique() refuses to work before seing SVM::~Impl().
        return std::unique_ptr< SVM >( new SVM( d ) );
    }
    if( is( "lda" ) )
    {
        return std::unique_ptr< LDAandSVM >( new LDAandSVM( d ) );
    }
    if( is( "pca" ) )
    {
        return std::unique_ptr< PCAandSVM >( new PCAandSVM( d ) );
    }
    if( is( "nn" ) )
    {
        return std::unique_ptr< NN >( new NN( d ) );
    }
#endif  // CMAKE_USE_DLIB
#ifdef CMAKE_USE_FOREST
    if( is( "forest" ) )
    {
        return std::unique_ptr< Forest >( new Forest( d ) );
    }
#endif

    throw Exception( name + ": no such model found. "
                     "See 'model.h' for a list of all models." );
}


extern const std::vector< std::string > ALL_MODELS;


}  // namespace model


#endif  // #ifndef MODEL_H_
