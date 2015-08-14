#pragma once

#include <pangolin/python/PyUniqueObj.h>
#include <pangolin/Console/ConsoleInterpreter.h>
#include <pangolin/compat/thread.h>
#include <queue>

namespace pangolin
{

class PyInterpreter : ConsoleInterpreter
{
public:
    PyInterpreter();

    ~PyInterpreter() PANGOLIN_OVERRIDE;

    void PushCommand(const std::string &cmd) PANGOLIN_OVERRIDE;

    bool PullLine(ConsoleLine& line) PANGOLIN_OVERRIDE;

    std::vector<std::string> Complete(
        const std::string& cmd, int max_options
    ) PANGOLIN_OVERRIDE;

private:
    std::string ToString(PyObject* py);
    PyUniqueObj EvalExec(const std::string& cmd);

    std::queue<ConsoleLine> line_queue;
};

}
