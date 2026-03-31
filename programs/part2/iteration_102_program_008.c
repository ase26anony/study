#ifndef GTY_TEST_HEADER_H
#define GTY_TEST_HEADER_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Core callback function pointer type with GTY((callback))
 * This is the essential type that will create TYPE_CALLBACK in gengtype's type table
 */
typedef void (*gty_callback_fn)(int, void*) GTY((callback));

/*
 * Another callback type with different signature
 */
typedef int (*gty_filter_fn)(const char*, size_t) GTY((callback));

/* 
 * TYPE_SCALAR: Simple scalar type with GTY annotation
 */
typedef unsigned long gty_scalar_type GTY((length));

/*
 * TYPE_STRUCT: Simple struct type
 */
struct GTY(()) gty_simple_struct {
    int field1;
    gty_scalar_type field2;
};

/*
 * TYPE_POINTER: Pointer type to the simple struct
 */
typedef struct gty_simple_struct* gty_struct_ptr GTY((tag("GTY_STRUCT_PTR")));

/*
 * TYPE_UNION: Union containing different types
 */
union GTY(()) gty_test_union {
    int int_val;
    gty_struct_ptr ptr_val;
    double double_val;
};

/*
 * TYPE_ARRAY: Struct containing an array
 */
struct GTY(()) gty_array_container {
    int numbers[20];
    char name[50];
};

/*
 * Struct containing callback function pointers - this will cause traversal
 * to the TYPE_CALLBACK when processing the struct
 */
struct GTY(()) gty_callback_container {
    /* Direct callback field */
    gty_callback_fn callback1;
    
    /* Array of callbacks */
    gty_filter_fn filters[3];
    
    /* Pointer to a struct containing a callback */
    struct gty_simple_struct* GTY((skip)) next;
    
    /* Union containing a callback */
    union {
        gty_callback_fn cb;
        int id;
    } GTY((desc("0"))) callback_union;
};

/*
 * More complex nested structure with callbacks
 */
struct GTY(()) gty_nested_callbacks {
    /* Container with callbacks */
    struct gty_callback_container container;
    
    /* Direct callback in nested struct */
    struct {
        gty_callback_fn notify;
        gty_filter_fn validate;
    } GTY((tag("NESTED"))) handlers;
    
    /* Pointer to callback (will be TYPE_POINTER -> TYPE_CALLBACK) */
    gty_callback_fn* callback_ptr;
};

/*
 * TYPE_USER_STRUCT: Forward declared struct that will be defined later
 */
struct GTY(()) gty_user_struct;

/*
 * Complete definition of user struct with callback
 */
struct GTY(()) gty_user_struct {
    int id;
    gty_callback_fn user_callback;
    struct gty_user_struct* GTY((skip)) next_user;
};

/*
 * Union specifically for testing TYPE_CALLBACK in union context
 */
union GTY(()) gty_callback_union {
    gty_callback_fn primary_cb;
    gty_filter_fn secondary_cb;
    void* GTY((tag("1"))) data;
};

/*
 * Struct with multiple callback types for comprehensive testing
 */
struct GTY(()) gty_multi_callback {
    /* Various callback declarations */
    void (*void_callback)(void) GTY((callback));
    int (*int_callback)(const char*) GTY((callback));
    size_t (*size_callback)(void*, size_t) GTY((callback));
    
    /* Regular fields */
    int counter;
    struct gty_multi_callback* GTY((skip)) chain;
};

/*
 * TYPE_LANG_STRUCT: Simulating a language-specific structure
 * In real GCC, these have special handling
 */
struct GTY(()) gty_lang_struct {
    int lang_specific;
    gty_callback_fn lang_callback;
};

/*
 * TYPE_STRING: String type (treated specially by GC)
 */
typedef const char* gty_string_type GTY((length));

/*
 * Final comprehensive struct using all types
 */
struct GTY(()) gty_comprehensive {
    /* TYPE_STRUCT */
    struct gty_simple_struct base;
    
    /* TYPE_UNION */
    union gty_test_union data;
    
    /* TYPE_ARRAY */
    int matrix[5][5];
    
    /* TYPE_CALLBACK (through pointer) */
    gty_callback_fn on_event;
    
    /* TYPE_STRING */
    gty_string_type name;
    
    /* TYPE_POINTER */
    gty_struct_ptr alias;
    
    /* TYPE_USER_STRUCT */
    struct gty_user_struct* users;
    
    /* TYPE_LANG_STRUCT */
    struct gty_lang_struct lang_data;
    
    /* TYPE_SCALAR */
    gty_scalar_type count;
};

#endif /* GTY_TEST_HEADER_H */
