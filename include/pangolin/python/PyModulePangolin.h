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
