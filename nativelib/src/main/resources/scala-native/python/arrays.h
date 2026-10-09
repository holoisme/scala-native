#if defined(SCALANATIVE_GC_PYTHON)

#include <Python.h>
#include "shared/MemoryMap.h"
#include "shared/ScalaNativeGC.h"

static Py_ssize_t scala_scalanative_runtime_Array_length(ArrayHeader *self) {
    return self->length;
}

static PyObject* scala_scalanative_runtime_Array_getitem(ArrayHeader *self, PyObject *key) {
    int index = PyLong_AsInt(key);
	if(index < 0 || index >= self->length) {
        return NULL;
    }

    PyObject* values = (PyObject*)(self + 1);

    return &values[index];
}

static PyObject* scala_scalanative_runtime_Array_item(ArrayHeader *self, Py_ssize_t index) {
	if(index < 0 || index >= self->length) {
        return NULL;
    }

    PyObject* values = (PyObject*)(self + 1);

    return &values[index];
}

static PyObject* scala_scalanative_runtime_IntArray_getitem(ArrayHeader *self, PyObject *key) {
    int index = PyLong_AsInt(key);
    if(index < 0 || index >= self->length) {
        return NULL;
    }

    int* values = (int*)(self + 1);

    return PyLong_FromInt32(values[index]);
}

static PyObject* scala_scalanative_runtime_IntArray_item(ArrayHeader *self, Py_ssize_t index) {
    if(index < 0 || index >= self->length) {
        return NULL;
    }

    int* values = (int*)(self + 1);
    return PyLong_FromInt32(values[index]);
}

static int scala_scalanative_runtime_Array_setitem(Object *self, PyObject *key, PyObject *value) {
    return 0;
}


static PyType_Slot scala_scalanative_runtime_Array[] = {
    { Py_sq_length, scala_scalanative_runtime_Array_length },
    { Py_mp_subscript, scala_scalanative_runtime_Array_getitem },
    { Py_mp_ass_subscript, scala_scalanative_runtime_Array_setitem },
    {0, NULL}
};

static PyType_Slot scala_scalanative_runtime_IntArray[] = {
    { Py_sq_length, scala_scalanative_runtime_Array_length },
    { Py_sq_item, scala_scalanative_runtime_IntArray_item },
    { Py_mp_subscript, scala_scalanative_runtime_IntArray_getitem },
    { Py_mp_ass_subscript, scala_scalanative_runtime_Array_setitem },
    {0, NULL}
};

#endif