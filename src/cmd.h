#ifndef CMD_H_
#define CMD_H_


// In this file: a list of all high-level actions.


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
            , const std::string & model_name );
    void execute() override;

private:
    const std::string _data_dir;
    const std::string _model_name;
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
