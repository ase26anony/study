/* gty-test-callbacks.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_TEST_CALLBACKS_H
#define GTY_TEST_CALLBACKS_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/* TYPE_SCALAR: Scalar type with GTY annotation */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer type to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_ARRAY: Struct containing an array */
struct GTY(()) array_container {
    int values[10];
    simple_ptr ptr;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    simple_ptr ptr_val;
    void* generic_ptr;
};

/* TYPE_CALLBACK in nested context: Callback inside a struct */
struct GTY(()) callback_container {
    /* Direct callback pointer */
    simple_callback_fn handler GTY((skip));
    
    /* Array of callbacks */
    simple_callback_fn handlers[3];
    
    /* Union containing a callback */
    union {
        simple_callback_fn callback;
        int mode;
    } GTY((desc("mode != 0"))) callback_union;
    
    /* Pointer to struct with callback */
    struct inner_callback {
        simple_callback_fn notify;
        int id;
    } GTY(()) *inner;
};

/* TYPE_CALLBACK: More complex callback signature */
typedef int (*complex_callback_fn)(
    const char* GTY((length)),
    simple_struct*,
    array_container*
) GTY((callback));

/* TYPE_STRUCT containing multiple callback types */
struct GTY(()) multi_callback_struct {
    /* Different callback types */
    simple_callback_fn simple_cb;
    complex_callback_fn complex_cb;
    
    /* Union with callback alternative */
    union GTY(()) {
        simple_callback_fn cb_fn;
        complex_callback_fn complex_fn;
        int (*other_fn)(void) GTY((callback));
    } callback_choice;
    
    /* Nested struct with callback */
    struct GTY(()) nested {
        simple_callback_fn nested_cb;
        int counter;
    } nested_data;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_specific {
    int lang_code;
    simple_callback_fn lang_handler;
    
    /* Tagged union for language variants */
    union GTY((tag("lang_code"))) {
        int c_mode;
        void* cpp_data;
    } variant;
};

/* TYPE_USER_STRUCT: Forward declared struct with callback */
struct user_defined;
typedef struct user_defined* user_ptr GTY((user));

struct GTY(()) user_defined {
    user_ptr next;
    simple_callback_fn user_callback;
    char* GTY((length)) name;
};

/* TYPE_CALLBACK with struct parameter containing callback */
typedef void (*recursive_callback_fn)(
    struct callback_container*,
    complex_callback_fn
) GTY((callback));

/* Final struct tying everything together */
struct GTY(()) master_container {
    /* All different types */
    simple_struct basic;
    array_container arrays;
    data_union union_data;
    callback_container callbacks;
    multi_callback_struct multi_callbacks;
    lang_specific lang;
    user_ptr user_chain;
    
    /* Callback that takes this struct as parameter */
    recursive_callback_fn recursive_cb;
    
    /* String type */
    const char* GTY((length)) description;
};

/* Additional callback-only type for pure TYPE_CALLBACK case */
typedef void (*final_callback_fn)(master_container*) GTY((callback));

#endif /* GTY_TEST_CALLBACKS_H */
