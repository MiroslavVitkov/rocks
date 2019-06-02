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


// Draw numbers from a RNG,
// following the distribution of the training labels.
// Don't look at the test data.
struct RandomChance : Base
{
    RandomChance( const std::string & data_dir = "./data/" );
    void execute() override;

private:
    const std::string _data_dir;
};


// Run cross corelation between training and test.
// Select the global maximal r_xy.
struct Correlation : Base
{
    Correlation( const std::string & data_dir = "./data/" );
    void execute() override;

private:
    const std::string _data_dir;
};


}  // namespace cmd


#endif  // defined(CMD_H_)
