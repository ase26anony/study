#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with parameters */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_SCALAR: Scalar type with GTY annotation */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct with GTY annotation */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer type with tag */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_union {
    int a;
    struct_ptr b;
    void* c;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) with_array {
    int arr[10];
    simple_callback_fn callbacks[5];
};

/* TYPE_CALLBACK nested in struct */
struct GTY(()) callback_container {
    /* Direct callback pointer */
    simple_callback_fn handler;
    
    /* Array of callbacks */
    complex_callback_fn handlers[3];
    
    /* Union containing callback */
    union {
        simple_callback_fn fn;
        int id;
    } GTY((desc("1"))) callback_union;
    
    /* Pointer to struct with callback */
    struct_ptr next;
};

/* TYPE_CALLBACK in typedef struct */
typedef struct GTY(()) {
    simple_callback_fn start;
    complex_callback_fn process;
    simple_callback_fn end;
} callback_suite;

/* Chain of structures with callbacks */
struct GTY(()) callback_chain {
    callback_suite suite;
    struct GTY((chain_next("%h.next"))) callback_chain* next;
};

/* TYPE_STRING: String type */
typedef const char* my_string GTY((length));

/* Struct mixing all types including callback */
struct GTY(()) mixed_types {
    my_string name;                 /* TYPE_STRING */
    my_scalar count;                /* TYPE_SCALAR */
    with_array data;                /* TYPE_STRUCT with TYPE_ARRAY */
    my_union choice;                /* TYPE_UNION */
    callback_container handlers;    /* TYPE_STRUCT with TYPE_CALLBACK */
    struct_ptr ptr;                 /* TYPE_POINTER */
};

#endif /* TEST_CALLBACK_TYPES_H */
