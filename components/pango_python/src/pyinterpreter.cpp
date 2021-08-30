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

void PyInterpreter::AttachPrefix(void* data, const std::string& name, const std::shared_ptr<VarValueGeneric>& /*var*/, bool /*brand_new*/ )
{
    PyInterpreter* self = (PyInterpreter*)data;

    const size_t dot = name.find_first_of('.');
    if(dot != std::string::npos) {
        const std::string base_prefix = name.substr(0,dot);
        if( self->base_prefixes.find(base_prefix) == self->base_prefixes.end() ) {
            self->base_prefixes.insert(base_prefix);
            std::string cmd =
                base_prefix + std::string(" = pypangolin.Var('") +
                base_prefix + std::string("')\n");
            PyRun_SimpleString(cmd.c_str());
        }
    }
}

PyInterpreter::PyInterpreter()
{
    using namespace py_pangolin;
    auto pypangolin = py::module_::import("pypangolin");
    
    auto sys = py::module_::import("sys");
    if(sys) {
        PyModule_AddObject(
            sys.ptr(), "stdout", (PyObject*)new PyPangoIO(
            &PyPangoIO::Py_type, line_queue, ConsoleLineTypeStdout
        ));
        PyModule_AddObject(
            sys.ptr(), "stderr", (PyObject*)new PyPangoIO(
            &PyPangoIO::Py_type, line_queue, ConsoleLineTypeStderr
        ));
     } else {
         pango_print_error("Couldn't import module sys.\n");
     }

    // // Attempt to setup readline completion
    // PyRun_SimpleString(
    //     "import pypangolin\n"
    //     "try:\n"
    //     "   import readline\n"
    //     "except ImportError:\n"
    //     "   import pyreadline as readline\n"
    //     "\n"
    //     "import rlcompleter\n"
    //     "pypangolin.completer = rlcompleter.Completer()\n"
    // );
    // CheckPrintClearError();

    // // Get reference to rlcompleter.Completer() for tab-completion
    // PyObject* mod_pangolin = PyImport_ImportModule("pypangolin");
    // if(mod_pangolin) {
    //     pycompleter = PyObject_GetAttrString(mod_pangolin,"completer");
    //     if(pycompleter) {
    //         pycomplete  = PyObject_GetAttrString(pycompleter,"complete");
    //     }
    // } else {
    //     pango_print_error("PyInterpreter: Unable to load module pangolin.\n");
    // }

    // // Hook namespace prefixes into Python
    // RegisterNewVarCallback(&PyInterpreter::AttachPrefix, (void*)this, "");
    // ProcessHistoricCallbacks(&PyInterpreter::AttachPrefix, (void*)this, "");

    // CheckPrintClearError();
}

PyInterpreter::~PyInterpreter()
{
//    Py_Finalize();
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
    auto ret = py::eval(cmd);
    CheckPrintClearError();
    return ret;
}

std::vector<std::string> PyInterpreter::Complete(const std::string& cmd, int max_options)
{
    std::vector<std::string> ret;
    PyErr_Clear();

//    if(pycomplete) {
//        for(int i=0; i < max_options; ++i) {
//#if PY_MAJOR_VERSION >= 3
//            PyUniqueObj args = PyTuple_Pack( 2, PyUnicode_FromString(cmd.c_str()), PyLong_FromSize_t(i) );
//            PyUniqueObj result = PyObject_CallObject(pycomplete, args);
//            if (result && PyUnicode_Check(result)) {
//                std::string res_str(PyUnicode_AsUTF8(result));
//#else
//            PyUniqueObj args = PyTuple_Pack(2, PyString_FromString(cmd.c_str()), PyInt_FromSize_t(i));
//            PyUniqueObj result = PyObject_CallObject(pycomplete, args);
//            if (result && PyString_Check(result)) {
//                std::string res_str(PyString_AsString(result));
//#endif
//                if( res_str.find("__")==std::string::npos ||
//                    cmd.find("__")!=std::string::npos ||
//                    (cmd.size() > 0 && cmd[cmd.size()-1] == '_')
//                ) {
//                    ret.push_back( res_str );
//                }
//            }else{
//                break;
//            }
//        }
//    }

    return ret;
}

void PyInterpreter::PushCommand(const std::string& cmd)
{
    auto obj = EvalExec(cmd);
    if(obj) {
        const std::string output = ToString(obj);
        line_queue.push(
            InterpreterLine(output, ConsoleLineTypeOutput)
        );
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

PANGOLIN_REGISTER_FACTORY(PyInterpreter)
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
