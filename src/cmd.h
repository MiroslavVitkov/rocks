#ifndef CMD_H_
#define CMD_H_


// In this file: a list of all high-level actions.


#include "dat.h"
#include "pre.h"

#include <memory>
#include <string>
#include <vector>


namespace cmd
{


struct Base
{
    virtual void execute() = 0;
    virtual ~Base() = default;
};


struct NoOp : Base
{
    void execute() override {}
};


struct RunModel : Base
{
    RunModel( const std::string & data_dir
            , const std::string & model_name
            , unsigned labels_depth  // see io.h
            , const std::vector< std::string > & preprocessing
            );
    void execute() override;
    std::pair< dat::Dataset, dat::Dataset > preprocess_dataset();
private:
    const std::string _data_dir;
    const std::string _model_name;
    const unsigned _labels_depth;
    const std::vector< std::string > _preprocessing;
};


struct RunAllModels : Base
{
    RunAllModels( const std::string & data_dir
                , unsigned labels_depth_max
                );
    void execute() override;

private:
    const std::string _data_dir;
    const unsigned _labels_depth_max;
};


struct ReportOutliers : Base
{
    ReportOutliers( const std::string & data_dir );
    void execute() override;

private:
    const std::string _data_dir;
};


}  // namespace cmd


#endif  // defined(CMD_H_)
