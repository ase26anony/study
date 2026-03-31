/* gty-test-header.h - Header file with GTY annotations for gengtype coverage */

#ifndef GTY_TEST_HEADER_H
#define GTY_TEST_HEADER_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_SCALAR: Scalar type with length attribute */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer type to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;
    simple_ptr c;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) with_array {
    int arr[10];
    simple_callback_fn callbacks[5];
};

/* TYPE_CALLBACK nested in struct: Callback inside a struct */
struct GTY(()) callback_container {
    simple_callback_fn handler;
    process_callback_fn processor;
    int id;
};

/* TYPE_CALLBACK in union */
union GTY(()) callback_union {
    simple_callback_fn fn;
    process_callback_fn proc;
    int handler_id;
};

/* More complex nested structure with callback */
struct GTY(()) complex_struct {
    struct GTY((tag("NESTED"))) nested {
        simple_callback_fn start;
        process_callback_fn end;
        int count;
    } inner;
    
    callback_container containers[3];
    my_union data;
};

/* TYPE_USER_STRUCT: Forward declaration that will be user-defined */
struct user_defined_struct;

/* Another struct referencing user-defined struct */
struct GTY(()) uses_user_struct {
    struct user_defined_struct* GTY((skip)) user_ptr;
    simple_callback_fn cleanup;
};

/* Array of callbacks */
typedef simple_callback_fn callback_array[10] GTY((tag("CALLBACK_ARRAY")));

/* Struct with pointer to callback */
struct GTY(()) indirect_callback {
    simple_callback_fn* callback_ptr;
    int priority;
};

/* Callback with struct parameter */
struct GTY(()) data_packet {
    int type;
    void* payload;
};

typedef void (*packet_callback_fn)(struct data_packet*) GTY((callback));

/* Final container with multiple callback types */
struct GTY(()) master_container {
    simple_callback_fn simple;
    process_callback_fn processor;
    packet_callback_fn packet_handler;
    callback_array callbacks;
    struct with_array arrays;
    union callback_union u;
};

#endif /* GTY_TEST_HEADER_H */
