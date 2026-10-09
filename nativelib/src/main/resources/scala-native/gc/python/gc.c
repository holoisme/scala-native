#if defined(SCALANATIVE_GC_PYTHON)

// sscanf and getEnv is deprecated in WinCRT, disable warnings
// These functions are not used directly, but are included in
// "shared/Parsing.h". The definition used to disable warnings needs to be
// placed before the first include of Windows.h, depending on the version of
// Windows runtime it might happen while preprocessing some of stdlib headers.
#define _CRT_SECURE_NO_WARNINGS

#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include "shared/MemoryMap.h"
#include "shared/MemoryInfo.h"
#include "shared/Parsing.h"
#include "shared/ScalaNativeGC.h"
#include "shared/Log.h"
#include <assert.h>
#include "python/slots.h"

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define GC_ASAN
#endif
#endif

int scalanative_bubbleExceptionToPython(Object* obj) {
    printf("[bubbleExceptionToPython] Step 1\n");
    fflush(stdout);
    PyObject *msg = PyUnicode_FromString("scala.SomeException: something went wrong");

    if (msg == NULL)
        return -1;

    PyErr_SetObject(PyExc_RuntimeError, msg);
    Py_DECREF(msg);

    return 0;
}

static size_t TOTAL_ALLOCATED = 0L; // Track total allocated memory

PyTypeObject* scalanative_PyType_fromRtti(Rtti* info);


PyTypeObject *scalanative_PyType_allocFromRtti(Rtti *rtti) {
    if (rtti == NULL || PyErr_Occurred() != NULL) {
        return NULL;
    }

    char *name = Rtti_name(rtti);
    fprintf(stderr, "Allocating type \"%s\"...\n", name);
    fflush(stderr);

    if (name == NULL) {
        return NULL;
    }

    PyType_Slot* slots = scalanative_PyType_decideSlots(rtti, name);

    PyType_Spec spec = {
        .name = name,
        .basicsize = rtti->size,
        .itemsize = 0,
        .flags =
            Py_TPFLAGS_DEFAULT |
            Py_TPFLAGS_BASETYPE |
            Py_TPFLAGS_DISALLOW_INSTANTIATION |
            Py_TPFLAGS_IMMUTABLETYPE,
        .slots = slots
    };

    PyObject *bases = NULL;

    if (rtti->superclass != NULL) {
        PyTypeObject *base = scalanative_PyType_fromRtti(rtti->superclass);

        if (base == NULL) {
            fprintf(stderr, "[ERROR PYTY] Base is null \"%s\" %d\n", name, rtti->size);
            fflush(stderr);
            free(name);
            return NULL;
        }

        bases = (PyObject *)base;
    }

    PyObject *obj = PyType_FromSpecWithBases(&spec, bases);

    Py_XDECREF(bases);

    // fprintf(stderr, " OK!\n");
    // fflush(stderr);

    return (PyTypeObject *)obj;
}

PyTypeObject* scalanative_PyType_fromRtti(Rtti* info) {
    if(info->pyType == NULL) {
        info->pyType = scalanative_PyType_allocFromRtti(info);
    }

    return info->pyType;
}

void *scalanative_GC_alloc(Rtti *info, size_t size) {
    size = (size + 7) & ~((size_t)7); // alignment

    Object *alloc = (Object *) PyObject_Calloc(1, size);
    // fprintf(stderr, "Allocation [%zu bytes]: %p\n", size, alloc);
    // fflush(stderr);

    PyTypeObject* py_type = scalanative_PyType_fromRtti(info);

    Py_SET_REFCNT(alloc, 1);
    Py_SET_TYPE(&alloc->py, py_type);


    alloc->rtti = info;
    TOTAL_ALLOCATED += size;
    return alloc;
}

void *scalanative_GC_alloc_small(Rtti *info, size_t size) {
    return scalanative_GC_alloc(info, size);
}

void *scalanative_GC_alloc_large(Rtti *info, size_t size) {
    return scalanative_GC_alloc(info, size);
}

void *scalanative_GC_alloc_array(Rtti *info, size_t length, size_t stride) {
    size_t size = info->size + length * stride;
    ArrayHeader *alloc = (ArrayHeader *)scalanative_GC_alloc(info, size);
    alloc->length = length;
    alloc->stride = stride;
    return alloc;
}

void scalanative_GC_collect() {}

void scalanative_GC_set_weak_references_collected_callback(
    WeakReferencesCollectedCallback callback) {}

#ifdef _WIN32
HANDLE scalanative_GC_CreateThread(LPSECURITY_ATTRIBUTES threadAttributes,
                                   SIZE_T stackSize, ThreadStartRoutine routine,
                                   RoutineArgs args, DWORD creationFlags,
                                   DWORD *threadId) {
    return CreateThread(threadAttributes, stackSize, routine, args,
                        creationFlags, threadId);
}
#else
int scalanative_GC_pthread_create(pthread_t *thread, pthread_attr_t *attr,
                                  ThreadStartRoutine routine,
                                  RoutineArgs args) {
    return pthread_create(thread, attr, routine, args);
}
#endif

// ScalaNativeGC interface stubs. None GC does not need STW
void scalanative_GC_set_mutator_thread_state(GC_MutatorThreadState unused) {}
void scalanative_GC_yield() {}
void scalanative_GC_add_roots(void *addr_low, void *addr_high) {}
void scalanative_GC_remove_roots(void *addr_low, void *addr_high) {}

static void exitWithOutOfMemory() {
    GC_LOG_ERROR("Out of heap space");
    exit(1);
}

size_t scalanative_GC_get_init_heapsize() {
    return Parse_Env_Or_Default("GC_INITIAL_HEAP_SIZE", 0L);
}

size_t scalanative_GC_get_max_heapsize() {
    return Parse_Env_Or_Default("GC_MAXIMUM_HEAP_SIZE", getMemorySize());
}

size_t scalanative_GC_get_used_heapsize() { return TOTAL_ALLOCATED; }

size_t scalanative_GC_stats_collection_total() { return -1L; }

size_t scalanative_GC_stats_collection_duration_total() { return -1L; }

void Prealloc_Or_Default() {}

void scalanative_GC_init() {
    GC_Log_Init();
    // Py_Initialize();
}

int scalanative_InitPySetType(Object *object) {
    Rtti* rtti = object->rtti;
    PyTypeObject* py_type = scalanative_PyType_fromRtti(rtti);

    if(py_type == NULL) {
        fprintf(stderr, "Couldn't initialize constant object\n");
        fflush(stderr);
        return 1;
    }

    Py_SET_REFCNT(object, PY_SSIZE_T_MAX);
    Py_SET_TYPE(&object->py, py_type);

    return 0;
}

#endif
