#ifndef TEST_GTY_CALLBACKS_H
#define TEST_GTY_CALLBACKS_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with parameters */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_SCALAR: Simple scalar type */
typedef unsigned int my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer to simple struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_UNION: Union containing various types */
union GTY(()) my_union {
    int a;
    simple_ptr b;
    void* c;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) with_array {
    int arr[10];
    simple_callback_fn callbacks[5];
};

/* TYPE_CALLBACK nested in struct */
struct GTY(()) callback_container {
    /* Callback function pointer field */
    simple_callback_fn handler GTY((tag("HANDLER")));
    
    /* Array of callbacks */
    process_callback_fn processors[3];
    
    /* Regular fields */
    int id;
    simple_ptr next;
};

/* TYPE_CALLBACK in union */
union GTY(()) callback_union {
    simple_callback_fn fn;
    process_callback_fn processor;
    int id;
    void* data;
};

/* TYPE_STRUCT with nested callback union */
struct GTY(()) nested_callback_struct {
    callback_union action;
    int state;
    struct nested_callback_struct* GTY((skip)) sibling;
};

/* TYPE_CALLBACK with struct parameter */
struct GTY(()) data_packet {
    int type;
    void* payload;
};

typedef void (*packet_callback_fn)(struct data_packet*) GTY((callback));

/* Complex struct mixing all types */
struct GTY(()) complex_type {
    /* TYPE_STRUCT embedded */
    simple_struct base;
    
    /* TYPE_UNION */
    my_union variant;
    
    /* TYPE_ARRAY */
    with_array items;
    
    /* TYPE_CALLBACK */
    packet_callback_fn packet_handler;
    
    /* TYPE_POINTER */
    callback_container* GTY((tag("CONTAINER_PTR"))) containers;
    
    /* TYPE_SCALAR */
    my_scalar count;
    
    /* TYPE_STRING (implicit through char*) */
    const char* GTY((tag("NAME"))) name;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_specific {
    int lang_code;
    void* GTY((skip)) lang_data;
    simple_callback_fn lang_callback;
};

/* Callback type that takes another callback as parameter */
typedef void (*chained_callback_fn)(simple_callback_fn) GTY((callback));

/* Final container with all callback types */
struct GTY(()) master_container {
    chained_callback_fn chain;
    callback_container direct;
    callback_union choice;
    complex_type complex;
    lang_specific lang;
};

#endif /* TEST_GTY_CALLBACKS_H */
