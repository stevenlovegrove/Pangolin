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
#include <Python.h>

#include <iomanip>
#include <queue>

#include <structmember.h>

#include <pangolin/var/var.h>
#include <pangolin/console/InterpreterInterface.h>

namespace py_pangolin
{

using namespace pangolin;

void bind_pango_write_object(pybind11::module& m);

struct PyPangoIO {
    PyObject_HEAD

    static PyTypeObject Py_type;
    static PyMethodDef Py_methods[];

    PyPangoIO(PyTypeObject *type, std::queue<InterpreterLine>& line_queue, InterpreterLineType line_type)
        : line_queue(line_queue), line_type(line_type)
    {
#if PY_MAJOR_VERSION >= 3
        ob_base.ob_refcnt = 1;
        ob_base.ob_type = type;
#else
        ob_refcnt = 1;
        ob_type = type;
#endif
    }

    static void Py_dealloc(PyPangoIO* self)
    {
        delete self;
    }

    static PyObject * Py_new(PyTypeObject */*type*/, PyObject */*args*/, PyObject */*kwds*/)
    {
        // Failure. Can only new in c++
        return 0;
    }

    static int Py_init(PyPangoIO* /*self*/, PyObject* /*args*/, PyObject* /*kwds*/)
    {
        return 0;
    }

    static PyObject* Py_getattr(PyPangoIO *self, char* name)
    {
#if PY_MAJOR_VERSION >= 3
        PyObject* pystr = PyUnicode_FromString(name);
#else
        PyObject* pystr = PyString_FromString(name);
#endif
        return PyObject_GenericGetAttr((PyObject*)self, pystr );
    }

    static int Py_setattr(PyPangoIO *self, char* name, PyObject* val)
    {
#if PY_MAJOR_VERSION >= 3
        PyObject* pystr = PyUnicode_FromString(name);
#else
        PyObject* pystr = PyString_FromString(name);
#endif
        return PyObject_GenericSetAttr((PyObject*)self, pystr, val);
    }

    static PyObject* Py_write(PyPangoIO* self, PyObject *args)
    {
        const char *text = 0;
        if (PyArg_ParseTuple(args, "s", &text)) {
            self->buffer += std::string(text);
            size_t nl = self->buffer.find_first_of('\n');
            while(nl != std::string::npos) {
                const std::string line = self->buffer.substr(0,nl);
                self->line_queue.push(InterpreterLine(line,self->line_type));
                self->buffer = self->buffer.substr(nl+1);
                nl = self->buffer.find_first_of('\n');
            }
        }
        Py_RETURN_NONE;
    }

    static PyObject* Py_flush(PyPangoIO* self, PyObject *args)
    {
        Py_RETURN_NONE;
    }

    std::string buffer;
    std::queue<InterpreterLine>& line_queue;
    InterpreterLineType line_type;
};




}
