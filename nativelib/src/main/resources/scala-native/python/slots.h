#if defined(SCALANATIVE_GC_PYTHON)

#include <Python.h>
#include "shared/MemoryMap.h"
#include <assert.h>

#include "object.h"
#include "arrays.h"
#include "option.h"

typedef struct {
    const char* name;
    PyType_Slot* slots;
} SlotEntry;

static PyType_Slot slots_none[] = {
    {0, NULL}
};

static SlotEntry ALL_SLOTS[] = {
    {
        .name = "java.lang.Object",
        .slots = java_lang_Object
    },
    {
        .name = "scala.scalanative.runtime.Array",
        .slots = scala_scalanative_runtime_Array
    },
    {
        .name = "scala.scalanative.runtime.IntArray",
        .slots = scala_scalanative_runtime_IntArray
    },
    {
        .name = "scala.Some",
        .slots = scala_Some
    },
    {
        .name = "scala.None$",
        .slots = scala_None$
    },

    {NULL}
};

PyType_Slot* scalanative_PyType_decideSlots(Rtti *rtti, const char* name) {
    SlotEntry* entry = ALL_SLOTS;

    while (entry->name != NULL) {
        if(strcmp(name, entry->name) == 0) {
            return entry->slots;
        }
        entry += 1;
    }

    return slots_none;
}

#endif
