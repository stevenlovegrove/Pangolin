#pragma once

#include <Python.h>
#include <structmember.h>
#include <iomanip>
#include <pangolin/var/var.h>

namespace pangolin
{

struct PyPangoVar {
    PyObject_HEAD
    PyPangoVar(PyTypeObject *type)
        : ob_refcnt(1), ob_type(type)
    {
    }

    std::string ns;
};

static void
PyPangoVar_dealloc(PyPangoVar* self)
{
//    self->ob_type->tp_free((PyObject*)self);
    delete self;
}

static PyObject *
PyPangoVar_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
//    // tp_alloc doesn't not correctly initialise c++ member variables.
//    PyPangoVar* self = (PyPangoVar *)type->tp_alloc(type, 0);
    PyPangoVar* self = new PyPangoVar(type);
    return (PyObject *)self;
}

static int
PyPangoVar_init(PyPangoVar *self, PyObject *args, PyObject *kwds)
{
    char* cNamespace = 0;
    if (!PyArg_ParseTuple(args, "s", &cNamespace))
        return -1;

    self->ns = std::string(cNamespace);

    return 0;
}

static PyMemberDef PyPangoVar_members[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef PyPangoVar_methods[] = {
    {NULL}  /* Sentinel */
};

static PyObject* PyPangoVar_getattr(PyPangoVar *self, char* name);
static int PyPangoVar_setattr(PyPangoVar *self, char* name, PyObject* val);

static PyTypeObject PyPangoVarType = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /*ob_size*/
    "pangolin.PangoVar",                      /*tp_name*/
    sizeof(PyPangoVar),                       /*tp_basicsize*/
    0,                                        /*tp_itemsize*/
    (destructor)PyPangoVar_dealloc,           /*tp_dealloc*/
    0,                                        /*tp_print*/
    (getattrfunc)PyPangoVar_getattr,          /*tp_getattr*/
    (setattrfunc)PyPangoVar_setattr,          /*tp_setattr*/
    0,                                        /*tp_compare*/
    0,                                        /*tp_repr*/
    0,                                        /*tp_as_number*/
    0,                                        /*tp_as_sequence*/
    0,                                        /*tp_as_mapping*/
    0,                                        /*tp_hash */
    0,                                        /*tp_call*/
    0,                                        /*tp_str*/
    0,                                        /*tp_getattro*/
    0,                                        /*tp_setattro*/
    0,                                        /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "PyPangoVar objects",                     /* tp_doc */
    0,		                              /* tp_traverse */
    0,		                              /* tp_clear */
    0,		                              /* tp_richcompare */
    0,		                              /* tp_weaklistoffset */
    0,		                              /* tp_iter */
    0,		                              /* tp_iternext */
    PyPangoVar_methods,                       /* tp_methods */
    PyPangoVar_members,                       /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)PyPangoVar_init,                /* tp_init */
    0,                                        /* tp_alloc */
    (newfunc)PyPangoVar_new,                  /* tp_new */
};


static PyObject* PyPangoVar_getattr(PyPangoVar *self, char* name)
{
    const std::string full_name = self->ns.empty() ? name : self->ns + "." + std::string(name);

    if( pangolin::VarState::I().Exists(full_name) ) {
        std::string str_val = pangolin::VarState::I()[full_name]->str->Get();
        return PyString_FromString(str_val.c_str());
    }else{
        PyPangoVar* obj = (PyPangoVar*)PyPangoVar_new(&PyPangoVarType,NULL,NULL);
        if(obj) {
            obj->ns = full_name;
            return PyObject_Init((PyObject *)obj,&PyPangoVarType);
        }

        return (PyObject *)obj;
    }
}

static int PyPangoVar_setattr(PyPangoVar *self, char* name, PyObject* val)
{
    const std::string full_name = self->ns.empty() ? name : self->ns + "." + std::string(name);

    PyUniqueObj pystr = PyObject_Repr(val);
    pangolin::Var<std::string> pango_var(full_name);
    pango_var = std::string(PyString_AsString(pystr));
    return 0;
}

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

static PyObject *PangoError;

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
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

PyMODINIT_FUNC InitPangoModule(void)
{
    if (PyType_Ready(&PyPangoVarType) < 0) {
        throw std::runtime_error("Unable to init object");
    }

    PyObject* m = Py_InitModule("pangolin", PangoMethods);
    if (m == NULL)
        return;

    PangoError = PyErr_NewException("pangolin.error", NULL, NULL);
    Py_INCREF(PangoError);
    PyModule_AddObject(m, "error", PangoError);

    Py_INCREF(&PyPangoVarType);
    PyModule_AddObject(m, "PangoVar", (PyObject *)&PyPangoVarType);
}

}
