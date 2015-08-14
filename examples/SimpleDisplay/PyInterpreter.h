#pragma once

#include <Python.h>
#include "PyPango.h"

namespace pangolin
{

class PythonInterpreter
{
public:
    PythonInterpreter()
    {
        InitFifo();
    }

    ~PythonInterpreter()
    {
        DeinitFifo();
    }

    void Send(const std::string& str)
    {
        write(fd_in, str.c_str(), str.size() );
    }

    void Send(const unsigned char ch)
    {
        write(fd_in, &ch, 1 );
    }

    std::string ToString(PyObject* py)
    {
        PyUniqueObj pystr = PyObject_Repr(py);
        return std::string( PyString_AsString(pystr) );
    }

    PyUniqueObj EvalExec(const std::string& cmd)
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
                            return PyObject_Call(eval, eval_args, 0);
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
                            return PyObject_Call(eval, eval_args, 0);
                        }
                    }
                }
            }
        }

        return PyUniqueObj();
    }

    std::string Complete(const std::string& str)
    {
        PyErr_Clear();

        std::string ret = str;
        PyObject* result = 0;

        PyObject* pymain = PyImport_AddModule("__main__");
        if(pymain) {
            PyObject* pycompleter = PyObject_GetAttrString(pymain,"pango_completer");
            if(pycompleter) {
                PyObject* pycomplete  = PyObject_GetAttrString(pycompleter,"complete");
                if(pycomplete) {
                    PyObject* args = PyTuple_Pack(2,PyString_FromString(str.c_str()), PyInt_FromSize_t(0));
                    result = PyObject_CallObject(pycomplete, args);
                    Py_DECREF(args);
                }
            }
        }

        if(result) {
            if(PyString_Check(result)) {
                ret = std::string( PyString_AsString(result) );
            }
            Py_DECREF(result);
        }

        return ret;
    }

private:
    void InitFifo()
    {
//        const char * fifo_in = "/tmp/pangolin_python_fifo_in";
//        const char * fifo_out = "/tmp/pangolin_python_fifo_out";

//        mkfifo(fifo_in, 0666);
//        // HANDLE hPipe = CreateNamedPipe(...) on windows, with _open_osfhandle for fd
//        fd_in = open(fifo_in, O_RDWR);
//        dup2(fd_in, 0); // stdin

//        mkfifo(fifo_out, 0666);
//        fd_out = open(fifo_out, O_RDWR);
//        dup2(fd_out, 1); // stdout

        Py_Initialize();

        InitPangoModule();

        PyRun_SimpleString(
            "import sys\n"
            "import pangolin\n"
            "\n"
//            "class PangoVar(object):\n"
//            "   def __init__(self, pango_ns):\n"
//            "       self.pango_ns = pango_ns\n"
//            "   \n"
//            "   def __getattr__(self, name):\n"
//            "       if(name == '__call__'):\n"
//            "           return []\n"
//            "       if(name == '__members__'):\n"
//            "           return ['A_Double']\n"
//            "       if(name == '__methods__'):\n"
//            "           return []\n"
//            "       print(self.pango_ns + name)\n"
//            "       return PangoVar(self.pango_ns + name)\n"
//            "\n"
//            "ui = PangoVar('ui')\n"
            "\n"
            "ui = pangolin.PangoVar('ui')\n"
            "\n"
            "try:\n"
            "   import readline\n"
            "except ImportError:\n"
            "   import pyreadline as readline\n"
            "\n"
            "import rlcompleter\n"
            "pango_completer = rlcompleter.Completer()\n"
        );

//        python_thread = std::thread(
//            [&](){
//                pangolin::BindToContext("Main");
//                PyRun_InteractiveLoop(stdin, "<stdin>");
//            }
//        );
    }

    void DeinitFifo()
    {
//        const std::string python_quit_string("\nquit()\n");
//        write(fd_in, python_quit_string.c_str(), python_quit_string.size());
//        python_thread.join();
        Py_Finalize();

//        close(fd_in);
//        close(fd_out);
    //    unlink(fifo_in);
    //    unlink(fifo_out);
    }

    void Read()
    {
        //////////////////////////////////////////////////
    //    std::string command("90+100+20\n");
    //    write(fd_in, command.c_str(), command.size() );

    //    int bytes_read = read(fd_out, buf, buf_size);
    //    if(bytes_read > 0) {
    //        std::cerr << "Recevied: " << std::string(buf, buf+bytes_read) << std::endl;
    //    }
    }

    int fd_in;
    int fd_out;
    std::thread python_thread;
};

}
