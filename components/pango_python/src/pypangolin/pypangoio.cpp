#include "pypangoio.h"

namespace py_pangolin
{
void bind_pango_write_object(pybind11::module& m) {
    Py_INCREF(&PyPangoIO::Py_type);
    PyModule_AddObject(m.ptr(), "PyPangoIO", (PyObject*)&PyPangoIO::Py_type);
}

PyMethodDef PyPangoIO::Py_methods[] = {
    {"write", (PyCFunction)PyPangoIO::Py_write, METH_VARARGS, "Write to console" },
    {"flush", (PyCFunction)PyPangoIO::Py_flush, METH_VARARGS, "flush outstanding writes" },
    {NULL, NULL, 0, NULL}
};

#pragma GCC diagnostic push // Ignore python missing initializer warning.
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

PyTypeObject PyPangoIO::Py_type = {
    PyVarObject_HEAD_INIT(NULL,0)
    "pypangolin.PangoIO",                     /* tp_name*/
    sizeof(PyPangoIO),                        /* tp_basicsize*/
    0,                                        /* tp_itemsize*/
    (destructor)PyPangoIO::Py_dealloc,        /* tp_dealloc*/
    0,                                        /* tp_print*/
    (getattrfunc)PyPangoIO::Py_getattr,       /* tp_getattr*/
    (setattrfunc)PyPangoIO::Py_setattr,       /* tp_setattr*/
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
    "PyPangoIO object",                       /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    PyPangoIO::Py_methods,                    /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)PyPangoIO::Py_init,             /* tp_init */
    0,                                        /* tp_alloc */
    (newfunc)PyPangoIO::Py_new,               /* tp_new */
    0,                                        /* tp_free */
    0,                                        /* tp_is_gc */
    0,                                        /* tp_bases */
    0,                                        /* tp_mro */
    0,                                        /* tp_cache */
    0,                                        /* tp_subclasses */
    0,                                        /* tp_weaklist */
    0,                                        /* tp_del */
    0                                         /* tp_version_tag */
};

#pragma GCC diagnostic pop  // Return to normal

}
