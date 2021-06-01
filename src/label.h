#ifndef LABEL_H_
#define LABEL_H_


// In this file:
//               1. typedefs for labels,
//               2. encoding string labels into ints.


#include <iostream>
#include <string>
#include <unordered_map>


namespace label
{


// Raw labels mimic directory structure. Ex: /azurite/spot02
// Every part of the hierarchical label starts with '/'.
using Raw = std::string;

// Numeric labels are efficient for processing and storage.
using Num = int;


struct Codec
{
    Num encode( const RawShort & l );
    Num encode( const RawShort & l ) const;
    const RawShort & decode( Num i ) const;

    friend std::ostream & operator<<( std::ostream &, const Codec & );
private:
    std::unordered_map< Raw, Num > _encoding;
    std::unordered_map< Num, Raw > _reverse;
};
std::ostream & operator<<( std::ostream &, const Codec & );


RawShort head( const Raw & );


}  // namespace label


#endif // defined( LABEL_H_ )
