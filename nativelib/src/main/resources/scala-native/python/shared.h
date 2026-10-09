#if defined(SCALANATIVE_GC_PYTHON)

#include <stdio.h>
#include <Python.h>
#include "shared/MemoryMap.h"
#include "shared/ScalaNativeGC.h"

int32_t cached_integer_id;

PyObject* scalanative_unboxToPython_fastpath(Object* val) {
    if(val->rtti->rt.id == cached_integer_id) {
        return PyLong_FromInt32(*val->fields[0]);
    }

    return NULL;
}

PyObject* scalanative_unboxToPython_slowpath(Object* val) {
    char *name = Rtti_name(val->rtti);
    if(!strcmp(name, "java.lang.Integer")) {
        cached_integer_id = val->rtti->rt.id;
        free(name);
        return PyLong_FromInt32(*(int*)&val->fields[0]);
    }

    return (PyObject*) val;
}

PyObject* scalanative_unboxToPython(Object* val) {
    PyObject* fast_result = scalanative_unboxToPython_fastpath(val);

    if(fast_result != NULL)
        return fast_result;

    return scalanative_unboxToPython_slowpath(val);
}

#endif
