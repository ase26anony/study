#ifndef TEST_GTY_CALLBACKS_H
#define TEST_GTY_CALLBACKS_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Function pointer with GTY((callback)) annotation
 * This is the key to triggering the uncovered case
 */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 
 * TYPE_SCALAR: Simple scalar type with GTY annotation
 */
typedef unsigned my_scalar GTY((length));

/*
 * TYPE_STRUCT: Simple struct with GTY annotation
 */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/*
 * TYPE_POINTER: Pointer type with GTY annotation
 */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/*
 * TYPE_UNION: Union with GTY annotation
 */
union GTY(()) my_union {
    int a;
    simple_ptr b;
    void* c;
};

/*
 * TYPE_ARRAY: Struct containing array with GTY annotation
 */
struct GTY(()) with_array {
    int arr[10];
    simple_callback_fn callbacks[5];
};

/*
 * Nested callback structure - callback inside a GTY struct
 * This ensures the callback type is discovered during traversal
 */
struct GTY(()) callback_container {
    /* Callback function pointer field */
    simple_callback_fn handler;
    
    /* Multiple callbacks in an array */
    complex_callback_fn handlers[3];
    
    /* Regular fields */
    int id;
    char* name;
};

/*
 * Union containing callback type
 */
union GTY(()) callback_union {
    simple_callback_fn fn;
    complex_callback_fn complex_fn;
    int id;
    void* data;
};

/*
 * More complex nested structure with callbacks
 */
struct GTY(()) nested_callback_struct {
    /* Direct callback field */
    simple_callback_fn direct_cb;
    
    /* Struct containing callback */
    callback_container container;
    
    /* Union with callback */
    callback_union cb_union;
    
    /* Array of structs with callbacks */
    callback_container container_array[2];
    
    /* Pointer to callback */
    simple_callback_fn* cb_ptr;
};

/*
 * Callback used in a typedef struct pattern
 */
struct GTY(()) callback_typedef_base {
    int base_field;
};

typedef struct callback_typedef_base* (*factory_callback_fn)(void) 
    GTY((callback));

/*
 * Struct using the factory callback
 */
struct GTY(()) uses_factory {
    factory_callback_fn create;
    void (*destroy)(struct callback_typedef_base*) GTY((callback));
};

/*
 * Self-referential structure with callback
 */
struct GTY(()) self_ref_with_callback {
    int value;
    simple_callback_fn processor;
    struct self_ref_with_callback* GTY((skip)) next;  /* skip to avoid cycles */
};

/*
 * Multiple callback types in one struct
 */
struct GTY(()) multi_callback {
    /* Different callback signatures */
    void (*cb1)(void) GTY((callback));
    int (*cb2)(int, char*) GTY((callback));
    char* (*cb3)(void*, size_t) GTY((callback));
    
    /* Mixed with regular data */
    int data;
    void* buffer;
};

#endif /* TEST_GTY_CALLBACKS_H */
