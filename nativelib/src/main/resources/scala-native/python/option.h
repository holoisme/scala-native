#if defined(SCALANATIVE_GC_PYTHON)

#include <Python.h>
#include "shared/MemoryMap.h"
#include "shared/ScalaNativeGC.h"
#include "shared.h"

static Py_ssize_t scala_Some_length(Object *self) {
    return 1;
}

static PyObject *scala_Some_get(Object *self, PyObject *_) {
    Object* value = (Object*) self->fields[0];
    return scalanative_unboxToPython(value);
}

static PyObject *scala_Some_is_some(Object *self, PyObject *_) {
    return PyBool_FromLong(1);
}

static PyObject *scala_Some_is_none(Object *self, PyObject *_) {
    return PyBool_FromLong(0);
}

static PyMethodDef scala_Some_methods[] = {
    { "get", (PyCFunction)scala_Some_get, METH_VARARGS, NULL },
    { "is_some", (PyCFunction)scala_Some_is_some, METH_VARARGS, NULL },
    { "is_none", (PyCFunction)scala_Some_is_none, METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};

static PyType_Slot scala_Some[] = {
    { Py_sq_length, scala_Some_length },
    { Py_tp_methods, scala_Some_methods },
    {0, NULL}
};


static Py_ssize_t scala_None$_length(Object *self) {
    return 0;
}

static PyObject *scala_None$_get(Object *self, PyObject *_) {
    PyObject *msg = PyUnicode_FromString("trying to unwrap an Option.None");

    if (msg == NULL)
        return NULL;

    PyErr_SetObject(PyExc_RuntimeError, msg);

    Py_DECREF(msg);

    return NULL;
}

static PyObject *scala_None$_is_some(Object *self, PyObject *_) {
    return PyBool_FromLong(0);
}

static PyObject *scala_None$_is_none(Object *self, PyObject *_) {
    return PyBool_FromLong(1);
}

static PyMethodDef scala_None$_methods[] = {
    { "get", (PyCFunction)scala_None$_get, METH_VARARGS, NULL },
    { "is_some", (PyCFunction)scala_None$_is_some, METH_VARARGS, NULL },
    { "is_none", (PyCFunction)scala_None$_is_none, METH_VARARGS, NULL },
    { NULL, NULL, 0, NULL }
};

static PyType_Slot scala_None$[] = {
    { Py_sq_length, scala_None$_length },
    { Py_tp_methods, scala_None$_methods },
    {0, NULL}
};

#endif
