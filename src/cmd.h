#ifndef CMD_H_
#define CMD_H_


// In this file: a list of all high-level actions.


#include <memory>
#include <string>


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


// See io.h for explaination of `labels_depth`.
struct RunModel : Base
{
    RunModel( const std::string & data_dir
            , const std::string & model_name
            , unsigned labels_depth );
    void execute() override;

private:
    const std::string _data_dir;
    const std::string _model_name;
    const unsigned _labels_depth;
};


// Run all avaible models at all label depths down to 1.
// Will take a very long time.
struct RunAll : Base
{
    RunAll( const std::string & data_dir
          , unsigned labels_depth_max );
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
