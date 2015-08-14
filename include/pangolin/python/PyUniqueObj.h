#pragma once

#include <pangolin/platform.h>

#include <Python.h>

namespace pangolin
{

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

}
