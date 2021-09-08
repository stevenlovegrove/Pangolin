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
#include <pangolin/python/pyinterpreter.h>
#include <pangolin/utils/file_utils.h>
#include <pangolin/var/varextra.h>
#include <pangolin/factory/factory_registry.h>

#include <pybind11/embed.h>
namespace py = pybind11;

#include "pypangolin/pypangoio.h"

namespace pangolin
{

void  PyInterpreter::NewVarCallback(const pangolin::VarState::Event& e)
{
    if(e.action == VarState::Event::Action::Added) {
        const std::string name = e.var->Meta().full_name;
        const size_t dot = name.find_first_of('.');
        if(dot != std::string::npos) {
            const std::string base_prefix = name.substr(0,dot);
            if( base_prefixes.find(base_prefix) == base_prefixes.end() ) {
                base_prefixes.insert(base_prefix);
                std::string cmd =
                    base_prefix + std::string(" = pypangolin.Var('") +
                    base_prefix + std::string("')\n");
                PyRun_SimpleString(cmd.c_str());
            }
        }
    }
}

PyInterpreter::PyInterpreter()
{
    using namespace py_pangolin;
    auto pypangolin = py::module_::import("pypangolin");
    
    auto sys = py::module_::import("sys");
    if(sys) {
        // TODO: What is the lifetime of PyPangoIO?
        PyPangoIO* wrap_stdout = new PyPangoIO(line_queue, ConsoleLineTypeStdout);
        PyPangoIO* wrap_stderr = new PyPangoIO(line_queue, ConsoleLineTypeStderr);
        sys.add_object("stdout", py::cast(wrap_stdout), true);
        sys.add_object("stderr", py::cast(wrap_stderr), true);
     } else {
         pango_print_error("Couldn't import module sys.\n");
     }

     // Attempt to setup readline completion
    py::exec(
         "import pypangolin\n"
         "try:\n"
         "   import readline\n"
         "except ImportError:\n"
         "   import pyreadline as readline\n"
         "\n"
         "import rlcompleter\n"
         "pypangolin.completer = rlcompleter.Completer()\n"
     );
     CheckPrintClearError();

     // Get reference to rlcompleter.Completer() for tab-completion
     pycompleter = pypangolin.attr("completer");
     pycomplete  = pycompleter.attr("complete");

     // Register for notifications on var additions
     var_added_connection = VarState::I().RegisterForVarEvents(
         std::bind(&PyInterpreter::NewVarCallback,this,std::placeholders::_1),
         true
     );

     CheckPrintClearError();

     // TODO: For some reason the completion will crash when the command contains '.'
     // unless we have executed anything that returns a value...
     // We probably have a PyRef issue, maybe?
     py::eval<py::eval_single_statement>("import sys");
     py::eval<py::eval_single_statement>("sys.version");
}

PyInterpreter::~PyInterpreter()
{
}

std::string PyInterpreter::ToString(const py::object& py)
{
    auto pystr = py::repr(py);
    return std::string(PyUnicode_AsUTF8(pystr.ptr()));
}

void PyInterpreter::CheckPrintClearError()
{
    if(PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
    }
}

py::object PyInterpreter::EvalExec(const std::string& cmd)
{
    py::object ret = py::none();

    if(!cmd.empty()) {
        try {
            ret = py::eval(cmd);
        }  catch (const pybind11::error_already_set& e) {
            line_queue.push(
                InterpreterLine(e.what(), ConsoleLineTypeStderr)
            );
        }
        CheckPrintClearError();
    }

    return ret;
}

std::vector<std::string> PyInterpreter::Complete(const std::string& cmd, int max_options)
{
    // TODO: When there is exactly 1 completion and it is smaller than our current string, we must invoke again with the prefix removed.

    std::vector<std::string> ret;
    PyErr_Clear();

    if(pycomplete) {
        for(int i=0; i < max_options; ++i) {
            auto args = PyTuple_Pack( 2, PyUnicode_FromString(cmd.c_str()), PyLong_FromSize_t(i) );
            auto result = PyObject_CallObject(pycomplete.ptr(), args);

            if (result && PyUnicode_Check(result)) {
                std::string res_str(PyUnicode_AsUTF8(result));
                ret.push_back( res_str );
                Py_DecRef(args);
                Py_DecRef(result);
            }else{
                Py_DecRef(args);
                Py_DecRef(result);
                break;
            }
        }
    }

    return ret;
}

void PyInterpreter::PushCommand(const std::string& cmd)
{
    if(!cmd.empty()) {
        try {
            py::eval<py::eval_single_statement>(cmd);
        }  catch (const pybind11::error_already_set& e) {
            line_queue.push(
                InterpreterLine(e.what(), ConsoleLineTypeStderr)
            );
        }
    }
}

bool PyInterpreter::PullLine(InterpreterLine& line)
{
    if(line_queue.size()) {
        line = line_queue.front();
        line_queue.pop();
        return true;
    }else{
        return false;
    }
}

PANGOLIN_REGISTER_FACTORY_WITH_STATIC_INITIALIZER(PyInterpreter)
{
    struct PyInterpreterFactory final : public TypedFactoryInterface<InterpreterInterface> {
        std::map<std::string,Precedence> Schemes() const override
        {
            return {{"python",10}};
        }
        const char* Description() const override
        {
            return "Python line interpreter.";
        }
        ParamSet Params() const override
        {
            return {{}};
        }
        std::unique_ptr<InterpreterInterface> Open(const Uri& /*uri*/) override {
            return std::unique_ptr<InterpreterInterface>(new PyInterpreter());
        }
    };

    return FactoryRegistry::I()->RegisterFactory<InterpreterInterface>(std::make_shared<PyInterpreterFactory>());
}

}
