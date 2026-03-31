#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*callback_func)(int) GTY((callback));

/* TYPE_STRUCT: Plain struct */
struct GTY(()) simple_struct {
    int field;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_ARRAY: Array within struct */
struct GTY(()) with_array {
    int arr[10];
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* Nested callback structure */
struct GTY(()) callback_container {
    /* Callback function pointer inside struct */
    callback_func handler GTY((skip));
    
    /* Array of callbacks */
    callback_func handlers[2];
    
    /* Union containing callback */
    union GTY(()) {
        callback_func fn;
        int id;
    } callback_union;
};

/* Another callback type for variety */
typedef int (*validate_func)(const char*) GTY((callback));

/* Struct mixing multiple callback types */
struct GTY(()) mixed_callbacks {
    callback_func cb1;
    validate_func cb2;
    
    /* Pointer to struct with callback */
    callback_container* GTY((tag("CONTAINER_PTR"))) container_ptr;
};

/* User-defined struct (TYPE_USER_STRUCT) */
struct GTY((user)) user_defined {
    int data;
};

#endif /* CALLBACK_TEST_H */
