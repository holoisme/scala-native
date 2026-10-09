#if defined(SCALANATIVE_GC_PYTHON)

#include <Python.h>
#include "shared/MemoryMap.h"
#include "shared/ScalaNativeGC.h"
#include <assert.h>

extern StringObject* scalanative_objectToString(Object* self);

static PyObject *java_lang_Object_toString(Object *self) {
    StringObject* str = scalanative_objectToString(self);
    uint16_t* chars = str->value->values;
    // PyObject* obj = PyUnicode_FromString(cstr);
    PyObject* obj = PyUnicode_DecodeUTF16(
        (const char *)chars,
        str->value->header.length * sizeof(uint16_t),
        NULL,
        NULL
    );
    // free(cstr);
    Py_DECREF(str);
    return obj;
}

static PyType_Slot java_lang_Object[] = {
    { Py_tp_str, java_lang_Object_toString },
    {0, NULL}
};

#endif
