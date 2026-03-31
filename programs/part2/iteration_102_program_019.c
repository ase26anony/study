/* gty-test-callbacks.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_TEST_CALLBACKS_H
#define GTY_TEST_CALLBACKS_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Core callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC operations */
struct GTY((user)) user_struct {
    int data;
    void (*cleanup)(struct user_struct*) GTY((skip));
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Pointer type with tag */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Struct containing arrays */
struct GTY(()) array_container {
    int numbers[10];
    simple_callback_fn callbacks[5];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_tag"))) lang_struct {
    int lang_tag;
    void* language_data;
};

/* TYPE_SCALAR: Scalar typedef with length attribute */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer type */
typedef const char* my_string GTY((string));

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
    simple_callback_fn handler;
    complex_callback_fn processor;
    int id;
};

/* Union containing callback */
union GTY(()) callback_union {
    simple_callback_fn fn_ptr;
    int callback_id;
    struct callback_container* container;
};

/* More complex nested structure with callback array */
struct GTY(()) nested_callback_struct {
    struct callback_container containers[3];
    union callback_union current_callback;
    my_string name;
    struct_ptr next;
};

/* Struct with multiple callback types */
struct GTY(()) multi_callback {
    /* Direct callback function pointer */
    simple_callback_fn simple_cb GTY((callback));
    
    /* Indirect through typedef */
    complex_callback_fn complex_cb;
    
    /* Array of callbacks */
    simple_callback_fn cb_array[4];
    
    /* Callback in union */
    union {
        simple_callback_fn cb1;
        complex_callback_fn cb2;
    } GTY((desc("%0.union_tag"))) cb_union;
    
    int union_tag;
};

/* Root structure that ties everything together */
struct GTY(()) root_struct {
    struct nested_callback_struct nested;
    struct multi_callback multi;
    union test_union data;
    struct lang_struct lang_data;
    my_scalar count;
    my_string description;
    struct array_container arrays;
};

/* Additional callback-only structure */
struct GTY(()) pure_callbacks {
    void (*startup)(void) GTY((callback));
    void (*shutdown)(void) GTY((callback));
    int (*process)(int, const char*) GTY((callback));
};

#endif /* GTY_TEST_CALLBACKS_H */
