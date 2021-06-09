#ifndef LABEL_H_
#define LABEL_H_


// In this file:
//               1. typedefs for labels,
//               2. encoding string labels into ints.


#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


namespace label
{


// Raw labels mimic directory structure. Ex: /azurite/spot02
// Every part of the hierarchical label starts with '/'.
using Raw = std::string;

// Numeric labels are efficient for processing and storage.
using Num = unsigned;


struct Codec
{
    Num encode( const Raw & l );
    Num encode( const Raw & l ) const;
    const Raw & decode( Num i ) const;

    // Respect only head labels (i.e. labels_depth == 1).
    Codec headonly() const;

    friend std::ostream & operator<<( std::ostream &, const Codec & );
private:
    std::unordered_map< Raw, Num > _encoding;
    std::unordered_map< Num, Raw > _reverse;
};
std::ostream & operator<<( std::ostream &, const Codec & );


Raw head( const Raw & );

std::vector< Num > headonly_recode( const std::vector< Num > &
                                  , const Codec & full
                                  , const Codec & headonly );


}  // namespace label


#endif // defined( LABEL_H_ )
