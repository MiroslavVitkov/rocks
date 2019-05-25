#ifndef CMD_H_
#define CMD_H_


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


struct Train : Base
{
    Train( const std::string & data_dir = "./data/" );
    void execute() override;

private:
    const std::string _data_dir;
};


}  // namespace cmd


#endif  // defined(CMD_H_)
