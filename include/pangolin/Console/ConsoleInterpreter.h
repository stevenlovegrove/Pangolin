#pragma once

#include <vector>
#include <string>

namespace pangolin
{

enum ConsoleLineType
{
    ConsoleLineTypeCmd,
    ConsoleLineTypeHistory,
    ConsoleLineTypeStdout,
    ConsoleLineTypeStderr,
    ConsoleLineTypeOutput,
    ConsoleLineTypeError,
};

class ConsoleLine
{
public:
    inline ConsoleLine()
        : linetype(ConsoleLineTypeCmd)
    {
    }

    inline ConsoleLine(std::string text, ConsoleLineType linetype = ConsoleLineTypeOutput)
        : text(text), linetype(linetype)
    {
    }

    std::string text;
    ConsoleLineType linetype;
};

class ConsoleInterpreter
{
public:
    inline virtual ~ConsoleInterpreter()
    {
    }

    virtual void PushCommand(const std::string& cmd) = 0;

    virtual bool PullLine(ConsoleLine& line) = 0;

    virtual std::vector<std::string> Complete(
        const std::string& cmd, int max_options = 20
    ) = 0;

};

}
