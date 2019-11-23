#include "dim.h"

#include "dat.h"

#include <dlib/statistics.h>


namespace dim
{
/*
std::vector<std::vector<int>> foo(std::vector<dat::Spectrum> &)
{
    using T = int;
    dlib::matrix< T > X;  // Each row of X is one input vector. Output Z in it.
    dlib::matrix< T, 0, 1 > M;  // Output.
    std::vector< unsigned long > row_labels;

    // Z*x-M maps x into a space where x vectors that share the same class label
    // are near each other.
    dlib::compute_lda_transform( X, M, row_labels, 1, 0 );

    return {};
}
*/

#include <dlib/statistics.h>
#include <iostream>

using namespace std;
using namespace dlib;

int main2(){
    typedef matrix<double,2,1> Sample;

    std::vector<unsigned long> labels;
    std::vector<Sample> samples;

    for (int i=0; i<4; i++){
        Sample s;
        s(0) = i;
        s(1) = i+1;
        samples.push_back(s);
        labels.push_back(1);

        Sample s1;
        s1(0) = i+1;
        s1(1) = i;
        samples.push_back(s1);
        labels.push_back(2);
    }

    matrix<double> X;
    X.set_size(8,2);
    for (unsigned i=0; i<8; i++){
        X(i,0) = samples[i](0);
        X(i,1) = samples[i](1);
    }

    matrix<double,0,1> mean;

    dlib::compute_lda_transform(X,mean,labels,1,0);

    for (unsigned i=0; i<samples.size(); i++){
        cout << X*samples[i]-mean << endl;
    }

    getchar();
    return 0;
}

}  // namespace dim
