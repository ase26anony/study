#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Core callback type definition */
typedef void (*simple_callback)(int) GTY((callback));

/* TYPE_STRUCT: Plain struct with callback field */
struct GTY(()) struct_with_callback {
    simple_callback cb;
    int data;
};

/* TYPE_UNION: Union containing callback */
union GTY(()) union_with_callback {
    simple_callback fn;
    int id;
    void* ptr;
};

/* TYPE_ARRAY: Array of callbacks */
struct GTY(()) struct_with_callback_array {
    simple_callback handlers[3];
    int count;
};

/* Nested callback structure */
typedef void (*nested_callback)(struct struct_with_callback*) GTY((callback));

/* TYPE_POINTER: Pointer type */
typedef struct struct_with_callback* callback_struct_ptr GTY((tag("CALLBACK_PTR")));

/* TYPE_USER_STRUCT: Forward declared struct with callback */
struct GTY(()) user_defined;
typedef void (*user_callback)(struct user_defined*) GTY((callback));

struct GTY(()) user_defined {
    user_callback notify;
    int value;
};

/* TYPE_STRING: String type */
typedef const char* my_string GTY((length("strlen(%h)")));

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* Complex nested structure with multiple callback types */
struct GTY(()) complex_container {
    simple_callback basic_cb;
    nested_callback nested_cb;
    union_with_callback callback_union;
    callback_struct_ptr next;
    my_string name;
    my_scalar id;
};

#endif /* CALLBACK_TEST_H */
