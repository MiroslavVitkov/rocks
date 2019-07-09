#ifndef IO_H_
#define IO_H_


// In this file: reading the dataset from disk.
//
// Example of the expected file structure.
// data                     <- arbitrary name
// ├── azurite              <- subdirs are labels
// │   ├── spot00           <- any subsubdirs are sublabels or flattened out
// │   │   ├── 10.csv
// │   │   ├── 11.csv
// │   │   ├── 12.csv
// │   │   ├── 14.csv
// │   │   ├── 15.csv
// │   │   ├── 16.csv
// │   │   ├── 19.csv
// │   │   └── 1.csv
// │   └── spot01
// │       └── 24.csv
// └── brochantite
//     ├── 99.csv           <- if we requested sublables, this
//     └── spot00              will generate an error
//         └── 7.csv
// Any non-directories or non .csv files are ignored.


#include "dat.h"

#include <string>


namespace io
{


// 'labels_depth == 1' -> '/azurite'
// 'labels_depth == 2' -> '/azurite/spot00' and an error for '99.csv'
dat::DataRaw read( const std::string & dataset_dir
                 , unsigned labels_depth = 1 );


}  // namespace io


#endif // defined( IO_H_ )
