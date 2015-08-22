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

#include <Python.h>
#include <pangolin/python/PyPangoVar.h>
#include <pangolin/var/var.h>

namespace pangolin
{

static PyObject * pango_GetVar(PyObject *self, PyObject *args)
{
    const char *var_name = 0;

    if (!PyArg_ParseTuple(args, "s", &var_name))
        return NULL;

    pangolin::Var<std::string> v(var_name);
    return Py_BuildValue("s", v.Get().c_str());
}

static PyMethodDef PangoMethods[] = {
    {"GetVar",  pango_GetVar, METH_VARARGS, "Get Pangolin Variable."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC InitPangoModule(void)
{
    if (PyType_Ready(&PyPangoVar::Py_type) >= 0) {
        PyObject* m = Py_InitModule("pangolin", PangoMethods);
        if (m) {
            Py_INCREF(&PyPangoVar::Py_type);
            PyModule_AddObject(m, "PangoVar", (PyObject *)&PyPangoVar::Py_type);
        }else{
            pango_print_error("Unable to initialise pangolin Python module.\n");
        }
    }else{
        pango_print_error("Unable to create pangolin Python objects.\n");
    }
}

}
