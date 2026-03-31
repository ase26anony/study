#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Basic callback function pointer type
 * This will create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/*
 * Another callback type with different signature
 */
typedef int (*process_data_fn)(const char*, size_t) GTY((callback));

/*
 * TYPE_STRUCT: Simple struct with scalar fields
 */
struct GTY(()) simple_struct {
    int field1;
    long field2;
    simple_callback_fn cb;  /* Contains a callback pointer */
};

/*
 * TYPE_UNION: Union containing various types including callbacks
 */
union GTY(()) callback_union {
    simple_callback_fn fn1;
    process_data_fn fn2;
    int id;
    void* GTY((skip)) ptr;  /* Skip this field for GC */
};

/*
 * TYPE_POINTER: Typedef for pointer type
 */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/*
 * TYPE_ARRAY: Struct containing arrays
 */
struct GTY(()) array_container {
    int numbers[10];
    simple_callback_fn handlers[5];  /* Array of callbacks */
    process_data_fn processors[3];
};

/*
 * TYPE_USER_STRUCT: Forward declaration that will be resolved
 */
struct GTY(()) user_defined;
typedef struct user_defined* user_ptr GTY((tag("USER_PTR")));

/*
 * Nested struct with callback at deeper level
 */
struct GTY(()) outer_container {
    struct GTY(()) inner {
        simple_callback_fn action;
        int trigger_value;
    } inner_struct;
    
    union GTY(()) choice {
        simple_callback_fn simple;
        process_data_fn complex;
    } callback_choice;
    
    array_container arrays;
};

/*
 * Struct with multiple callback types mixed with other fields
 */
struct GTY(()) mixed_callbacks {
    /* Scalar fields */
    int id;
    unsigned count;
    
    /* Callback fields */
    simple_callback_fn on_start;
    process_data_fn on_data;
    simple_callback_fn on_end GTY((skip));  /* Skip this callback */
    
    /* Pointer to another GTY type */
    struct_ptr next;
    
    /* Union with callback */
    callback_union union_field;
};

/*
 * More complex: Struct containing struct with callback
 */
struct GTY(()) callback_wrapper {
    struct GTY(()) {
        simple_callback_fn handler;
        char* GTY((length)) name;
    } named_handler;
    
    int priority;
};

/*
 * TYPE_LANG_STRUCT: Simulating a language-specific structure
 * (In real GCC, these would be in language-specific headers)
 */
struct GTY(()) lang_specific {
    int lang_code;
    simple_callback_fn lang_handler;
};

/*
 * TYPE_STRING: String type (though usually handled differently)
 * Using length attribute for string
 */
typedef char* gty_string GTY((length("strlen(%h) + 1")));

/*
 * Struct using string type with callback
 */
struct GTY(()) string_and_callback {
    gty_string message;
    simple_callback_fn logger;
};

/*
 * Recursive structure with callback
 */
struct GTY(()) tree_node {
    int value;
    simple_callback_fn visitor;
    struct tree_node* GTY((skip("skip"))) left;  /* Skip for now */
    struct tree_node* GTY((skip("skip"))) right; /* Skip for now */
};

/*
 * Union with multiple callback options
 */
union GTY(()) multi_callback_union {
    simple_callback_fn simple;
    process_data_fn processor;
    void (*void_fn)(void) GTY((callback));
};

/*
 * Final comprehensive test structure
 */
struct GTY(()) comprehensive_test {
    /* All different kinds of fields */
    int scalar_field;                     /* TYPE_SCALAR */
    gty_string string_field;              /* TYPE_STRING */
    simple_struct struct_field;           /* TYPE_STRUCT */
    callback_union union_field;           /* TYPE_UNION */
    struct_ptr pointer_field;             /* TYPE_POINTER */
    simple_callback_fn callback_field;    /* TYPE_CALLBACK */
    array_container array_field;          /* TYPE_ARRAY */
    lang_specific lang_field;             /* TYPE_LANG_STRUCT */
    
    /* Array of mixed types including callbacks */
    simple_callback_fn callback_array[3];
    process_data_fn processor_array[2];
    
    /* Nested anonymous struct with callback */
    struct GTY(()) {
        simple_callback_fn nested_cb;
        int nested_data;
    } anonymous;
};

#endif /* GTY_CALLBACK_TEST_H */
