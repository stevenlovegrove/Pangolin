#pragma once

#include <pangolin/python/PyUniqueObj.h>
#include <string>
#include <thread>

namespace pangolin
{

class PythonInterpreter
{
public:
    PythonInterpreter();

    ~PythonInterpreter();

    std::string ToString(PyObject* py);

    PyUniqueObj EvalExec(const std::string& cmd);

    std::string Complete(const std::string& str);

private:
    std::thread python_thread;
};

}
