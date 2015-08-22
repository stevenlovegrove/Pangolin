#pragma once

#include <Python.h>
#include <structmember.h>
#include <iomanip>
#include <pangolin/var/var.h>

namespace pangolin
{

struct PyPangoVar {
    static PyTypeObject Py_type;
    PyObject_HEAD

    PyPangoVar(PyTypeObject *type)
        : ob_refcnt(1), ob_type(type)
    {
    }

    static void Py_dealloc(PyPangoVar* self)
    {
        delete self;
    }

    static PyObject * Py_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
    {
        PyPangoVar* self = new PyPangoVar(type);
        return (PyObject *)self;
    }

    static int Py_init(PyPangoVar *self, PyObject *args, PyObject *kwds)
    {
        char* cNamespace = 0;
        if (!PyArg_ParseTuple(args, "s", &cNamespace))
            return -1;

        self->ns = std::string(cNamespace);

        return 0;
    }

    static PyObject* Py_getattr(PyPangoVar *self, char* name)
    {
        const std::string prefix = self->ns + ".";
        const std::string full_name = self->ns.empty() ? name : prefix + std::string(name);

        if( !strcmp(name, "__call__") ||
                !strcmp(name, "__dict__") ||
                !strcmp(name, "__methods__") ||
                !strcmp(name, "__class__") )
        {
            // Default behaviour
            return PyObject_GenericGetAttr((PyObject*)self, PyString_FromString(name));
        } else if( !strcmp(name, "__members__") ) {
            const int nss = prefix.size();
            PyObject* l = PyList_New(0);
            for(const std::string& s : VarState::I().var_adds) {
                if(!s.compare(0, nss, prefix)) {
                    int dot = s.find_first_of('.', nss);
                    if(dot != std::string::npos) {
                        std::string val = s.substr(nss, dot - nss);
                        PyList_Append(l, PyString_FromString(val.c_str()));
                    }else{
                        std::string val = s.substr(nss);
                        PyList_Append(l, PyString_FromString(val.c_str()));
                    }
                }
            }

            return l;
        }else if( pangolin::VarState::I().Exists(full_name) ) {
            std::string str_val;
            try{
                str_val = pangolin::VarState::I()[full_name]->str->Get();
            }catch(pangolin::BadInputException) {
            }
            return PyString_FromString(str_val.c_str());
        }else{
            PyPangoVar* obj = (PyPangoVar*)PyPangoVar::Py_new(&PyPangoVar::Py_type,NULL,NULL);
            if(obj) {
                obj->ns = full_name;
                return PyObject_Init((PyObject *)obj,&PyPangoVar::Py_type);
            }
            return (PyObject *)obj;
        }

        Py_RETURN_NONE;
    }

    static int Py_setattr(PyPangoVar *self, char* name, PyObject* val)
    {
        const std::string full_name = self->ns.empty() ? name : self->ns + "." + std::string(name);

        PyUniqueObj pystr = PyObject_Repr(val);
        pangolin::Var<std::string> pango_var(full_name);
        pango_var = std::string(PyString_AsString(pystr));
        return 0;
    }

    std::string ns;
};

 PyTypeObject PyPangoVar::Py_type = {
    PyObject_HEAD_INIT(NULL)
    0,                                        /* ob_size*/
    "pangolin.PangoVar",                      /* tp_name*/
    sizeof(PyPangoVar),                       /* tp_basicsize*/
    0,                                        /* tp_itemsize*/
    (destructor)PyPangoVar::Py_dealloc,       /* tp_dealloc*/
    0,                                        /* tp_print*/
    (getattrfunc)PyPangoVar::Py_getattr,      /* tp_getattr*/
    (setattrfunc)PyPangoVar::Py_setattr,      /* tp_setattr*/
    0,                                        /* tp_compare*/
    0,                                        /* tp_repr*/
    0,                                        /* tp_as_number*/
    0,                                        /* tp_as_sequence*/
    0,                                        /* tp_as_mapping*/
    0,                                        /* tp_hash */
    0,                                        /* tp_call*/
    0,                                        /* tp_str*/
    0,                                        /* tp_getattro*/
    0,                                        /* tp_setattro*/
    0,                                        /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
    "PyPangoVar object",                      /* tp_doc */
    0,		                              /* tp_traverse */
    0,		                              /* tp_clear */
    0,		                              /* tp_richcompare */
    0,		                              /* tp_weaklistoffset */
    0,		                              /* tp_iter */
    0,		                              /* tp_iternext */
    0,                                        /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)PyPangoVar::Py_init,            /* tp_init */
    0,                                        /* tp_alloc */
    (newfunc)PyPangoVar::Py_new,              /* tp_new */
};

}
