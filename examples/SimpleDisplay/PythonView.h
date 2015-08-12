#pragma once

#include <deque>

#include <pangolin/platform.h>
#include <pangolin/gl/glfont.h>
#include <pangolin/var/var.h>

#include <Python.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>

namespace pangolin
{

static PyObject *PangoError;

static PyObject * pango_GetVar(PyObject *self, PyObject *args)
{
    const char *var_name;

    if (!PyArg_ParseTuple(args, "s", &var_name))
        return NULL;

    pangolin::Var<std::string> v(var_name);
    return Py_BuildValue("s", v.Get().c_str());
}

static PyMethodDef PangoMethods[] = {
    {"GetVar",  pango_GetVar, METH_VARARGS, "Get Pangolin Variable."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC InitPangoModule(void)
{
    PyObject *m;

    m = Py_InitModule("pango", PangoMethods);
    if (m == NULL)
        return;

    PangoError = PyErr_NewException("pango.error", NULL, NULL);
    Py_INCREF(PangoError);
    PyModule_AddObject(m, "error", PangoError);
}

struct PyUniqueObj
{
public:
    PyUniqueObj()
        : obj(0)
    {
    }

    PyUniqueObj(PyObject* obj)
        : obj(obj)
    {
    }

    ~PyUniqueObj()
    {
        if(obj) Py_DECREF(obj);
    }

#ifdef CALLEE_HAS_RVALREF
    PyUniqueObj(PyUniqueObj&& other)
        : obj(other.obj)
    {
        other.obj = 0;
    }
#endif

    inline void Dec() {
        if(obj) {
            Py_DECREF(obj);
            obj = 0;
        }
    }

    inline PyObject* operator*() {
        return obj;
    }

    inline operator PyObject*() {
        return obj;
    }

private:
    // Private copy constructor
    PyUniqueObj(const PyUniqueObj&) {}

    PyObject* obj;
};

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
            "import pango\n"
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




class PythonView : public pangolin::View, pangolin::Handler
{
public:
    PythonView()
        : font(GlFont::I()),
          prompt(font.Text("")),
          current_line(prompt)
    {
        SetHandler(this);
        AddLine("Pangolin Python Command Prompt:");
        AddLine("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
    }

    ~PythonView() {
    }

    void Render()
    {
        this->ActivatePixelOrthographic();
        glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TRANSFORM_BIT );

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
        glDisable(GL_DEPTH_TEST );

        glColor4f( 1.0, 0.5, 0.5, 0.8 );

        GLfloat verts[] = { 0.0f, (GLfloat)v.h,
                            (GLfloat)v.w, (GLfloat)v.h,
                            (GLfloat)v.w, 0.0f,
                            0.0f, 0.0f };
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, verts);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);


        const int line_space = 15;
        glTranslated(10.0, 10.0, 0.0 );
        glColor4f( 1.0, 1.0, 1.0, 1.0 );
        current_line.Draw();
        glTranslated(0.0, line_space, 0.0);
        for(int l=0; l < line_buffer.size(); ++l) {
            GlText& txt = line_buffer[l];
            txt.Draw();
            glTranslated(0.0, line_space, 0.0);
        }

        glPopAttrib();
    }

    void Keyboard(View&, unsigned char key, int x, int y, bool pressed)
    {
        if(pressed) {
            if(key=='\r') key = '\n';

            if(key=='\n') {
                const std::string cmd = current_line.Text();

                PyUniqueObj obj = python.EvalExec(cmd);
                line_buffer.push_front(current_line);
                if(obj) {
                    // Succeeded
                    if(obj != Py_None) {
                        // With result
                        AddLine(python.ToString(obj));
                    }
                }
                current_line = prompt;

            }else if(key=='\t') {
                current_line = font.Text("%s", python.Complete(current_line.Text()).c_str());
            }else if(key=='\b') {
                current_line = font.Text("%s", current_line.Text().substr(0,current_line.Text().size()-1).c_str() );
            }else{
                current_line = font.Text("%s%c", current_line.Text().c_str(), key);
            }
        }
    }

    void Mouse(pangolin::View&, pangolin::MouseButton button, int x, int y, bool pressed, int button_state)
    {
    }

    void MouseMotion(pangolin::View&, int x, int y, int button_state)
    {
    }

    void PassiveMouseMotion(pangolin::View&, int x, int y, int button_state)
    {
    }

    void Special(pangolin::View&, pangolin::InputSpecial inType, float x, float y, float p1, float p2, float p3, float p4, int button_state)
    {
    }

    void AddLine(const std::string& str)
    {
        line_buffer.push_front( font.Text("%s",str.c_str()) );
    }

private:
    PythonInterpreter python;

    GlFont& font;
    GlText prompt;
    GlText current_line;
    std::deque<GlText> line_buffer;
};

}
