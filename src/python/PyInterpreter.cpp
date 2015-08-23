#include <pangolin/python/PyInterpreter.h>
#include <pangolin/python/PyUniqueObj.h>

#include <Python.h>
#include <pangolin/python/PyModulePangolin.h>
#include <pangolin/python/PyInterpreter.h>
#include <pangolin/python/PyPangoIO.h>


namespace pangolin
{

void PyInterpreter::AttachPrefix(void* data, const std::string& name, VarValueGeneric& /*var*/, bool /*brand_new*/ )
{
    PyInterpreter* self = (PyInterpreter*)data;

    const int dot = name.find_first_of('.');
    if(dot != std::string::npos) {
        const std::string base_prefix = name.substr(0,dot);
        if( self->base_prefixes.find(base_prefix) == self->base_prefixes.end() ) {
            self->base_prefixes.insert(base_prefix);
            std::string cmd =
                base_prefix + std::string(" = pangolin.PangoVar('") +
                base_prefix + std::string("')\n");
            PyRun_SimpleString(cmd.c_str());
        }
    }
}

PyInterpreter::PyInterpreter()
{
    Py_Initialize();

    InitPangoModule();

    PyRun_SimpleString(
        "import pangolin\n"
        "try:\n"
        "   import readline\n"
        "except ImportError:\n"
        "   import pyreadline as readline\n"
        "\n"
        "import rlcompleter\n"
        "pango_completer = rlcompleter.Completer()\n"
    );

    // Hook namespace prefixes into Python
    RegisterNewVarCallback(&PyInterpreter::AttachPrefix,(void*)this,"");
    ProcessHistoricCallbacks(&PyInterpreter::AttachPrefix,(void*)this,"");

    PyObject* mod_sys = PyImport_ImportModule("sys");
    if(mod_sys) {
        PyModule_AddObject( mod_sys, "stdout", (PyObject*)new PyPangoIO(
                &PyPangoIO::Py_type, line_queue, ConsoleLineTypeStdout
        ) );
        PyModule_AddObject( mod_sys, "stderr", (PyObject*)new PyPangoIO(
                &PyPangoIO::Py_type, line_queue, ConsoleLineTypeStderr
        ) );
    }else{
        pango_print_error("Couldn't import module");
    }

}

PyInterpreter::~PyInterpreter()
{
    Py_Finalize();
}

std::string PyInterpreter::ToString(PyObject* py)
{
    PyUniqueObj pystr = PyObject_Repr(py);
    return std::string( PyString_AsString(pystr) );
}

void PyInterpreter::CheckPrintClearError()
{
    if(PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
    }
}

PyUniqueObj PyInterpreter::EvalExec(const std::string& cmd)
{
    PyObject* globals = PyModule_GetDict(PyImport_AddModule("__main__"));
    PyObject* builtin = PyImport_AddModule("__builtin__");

    if(builtin) {
        PyUniqueObj compile = PyObject_GetAttrString(builtin, "compile");
        PyUniqueObj eval = PyObject_GetAttrString(builtin, "eval");

        if(compile && eval)
        {
            PyErr_Clear();
            PyUniqueObj compile_eval_args = Py_BuildValue("(sss)", cmd.c_str(), "<string>", "eval" );
            if(compile_eval_args)
            {
                PyUniqueObj code = PyObject_Call(compile, compile_eval_args, 0);
                if(code) {
                    PyUniqueObj eval_args = Py_BuildValue("(OOO)", *code, globals, globals );
                    if(eval_args) {
                        PyUniqueObj ret = PyObject_Call(eval, eval_args, 0);
                        CheckPrintClearError();
                        return ret;
                    }
                }
            }

            PyErr_Clear();
            PyUniqueObj compile_exec_args = Py_BuildValue("(sss)", cmd.c_str(), "<string>", "exec" );
            if(compile_exec_args)
            {
                PyUniqueObj code = PyObject_Call(compile, compile_exec_args, 0);
                if(code) {
                    PyUniqueObj eval_args = Py_BuildValue("(OOO)", *code, globals, globals );
                    if(eval_args) {
                        PyUniqueObj ret = PyObject_Call(eval, eval_args, 0);
                        CheckPrintClearError();
                        return ret;
                    }
                }
            }
        }
    }

    CheckPrintClearError();
    return PyUniqueObj();
}

std::vector<std::string> PyInterpreter::Complete(const std::string& cmd, int max_options)
{
    std::vector<std::string> ret;
    PyErr_Clear();

    PyObject* pymain = PyImport_AddModule("__main__");
    if(pymain) {
        PyObject* pycompleter = PyObject_GetAttrString(pymain,"pango_completer");
        if(pycompleter) {
            PyObject* pycomplete  = PyObject_GetAttrString(pycompleter,"complete");
            if(pycomplete) {
                for(int i=0; i < max_options; ++i) {
                    PyUniqueObj args = PyTuple_Pack( 2, PyString_FromString(cmd.c_str()), PyInt_FromSize_t(i) );
                    PyUniqueObj result = PyObject_CallObject(pycomplete, args);
                    if(result && PyString_Check(result) ) {
                        std::string res_str( PyString_AsString(result) );
                        if( res_str.find("__")==std::string::npos ||
                            cmd.find("__")!=std::string::npos ||
                            (cmd.size() > 0 && cmd[cmd.size()-1] == '_')
                        ) {
                            ret.push_back( res_str );
                        }
                    }else{
                        break;
                    }
                }
            }
        }
    }

    return ret;
}

void PyInterpreter::PushCommand(const std::string& cmd)
{
    PyUniqueObj obj = EvalExec(cmd);
    if(obj && obj != Py_None) {
        const std::string output = ToString(obj);
        line_queue.push(
            ConsoleLine(output, ConsoleLineTypeOutput)
        );
    }
}

bool PyInterpreter::PullLine(ConsoleLine& line)
{
    if(line_queue.size()) {
        line = line_queue.front();
        line_queue.pop();
        return true;
    }else{
        return false;
    }
}

}
