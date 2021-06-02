#ifndef TASK_H_
#define TASK_H_


// In this file: multithreaded model evaluation; training is far too specific to generalise so.


#include "dat.h"
#include "label.h"
#include "model.h"

#include <vector>
#include <future>


namespace task
{


struct Task
{
    Task( const model::Model &, const std::vector< dat::Spectrum > & );
    std::vector< label::Num > get();

    Task( Task && t );
    Task( const Task & ) = delete;
    ~Task() = default;
    Task & operator=( Task && other) = delete;
    Task & operator=(const Task & other) = delete;


private:
    std::promise< std::vector<label::Num> > _p;
    std::thread _t;
};


}  // namespace task


#endif  // TASK_H_
