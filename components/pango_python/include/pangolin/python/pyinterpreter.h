/* This file is part of the Pangolin Project.
 * http://github.com/stevenlovegrove/Pangolin
 *
 * Copyright (c) 2011 Steven Lovegrove
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <pybind11/embed.h>
#include <pangolin/var/varextra.h>
#include <pangolin/console/InterpreterInterface.h>
#include <queue>
#include <set>
#include <thread>

namespace pangolin
{

class PyInterpreter : public InterpreterInterface
{
public:
    PyInterpreter();

    ~PyInterpreter() override;

    void PushCommand(const std::string &cmd) override;

    bool PullLine(InterpreterLine& line) override;

    std::vector<std::string> Complete(
        const std::string& cmd, int max_options
    ) override;

private:
    void  NewVarCallback(const pangolin::VarState::Event& e);

    pybind11::scoped_interpreter guard;

    pybind11::object pycompleter;
    pybind11::object pycomplete;

    std::string ToString(const pybind11::object& py);
    pybind11::object EvalExec(const std::string& cmd);
    void CheckPrintClearError();

    std::queue<InterpreterLine> line_queue;
    std::set<std::string> base_prefixes;
    sigslot::scoped_connection var_added_connection;
};

}
