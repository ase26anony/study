/* gty-callback-test.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* Basic callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* Union type */
union GTY(()) my_union {
    int int_val;
    void* GTY((tag("UNION_PTR"))) ptr_val;
    double double_val;
};

/* Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* Array type within a struct */
struct GTY(()) array_container {
    int numbers[10];
    char GTY((length("strlen(name)+1"))) *name;
};

/* Scalar typedef with GTY marker */
typedef unsigned long my_scalar GTY((length));

/* Struct containing a callback function pointer */
struct GTY(()) callback_container {
    simple_callback_fn handler;
    process_callback_fn processor;
    int id;
};

/* More complex nested structure with callback */
struct GTY(()) nested_callback_struct {
    struct callback_container GTY((tag("NESTED_CONTAINER"))) *container;
    simple_callback_fn GTY((skip)) extra_callback;  /* skip marker for variety */
    union my_union data;
};

/* Array of callbacks */
struct GTY(()) callback_array {
    simple_callback_fn handlers[5];
    int count;
};

/* Union containing callback */
union GTY(()) callback_union {
    simple_callback_fn fn;
    process_callback_fn processor;
    int handler_id;
};

/* Struct with callback pointer array */
struct GTY(()) callback_ptr_array {
    simple_callback_fn* GTY((length("callback_count"))) callbacks;
    int callback_count;
};

/* Typedef struct with callback */
typedef struct GTY(()) {
    simple_callback_fn init;
    process_callback_fn run;
    void (*cleanup)(void);  /* Not GTY-marked - for contrast */
} callback_module;

/* Another callback type definition */
typedef void (*finalize_callback_fn)(void*) GTY((callback));

/* Complex structure mixing all types */
struct GTY(()) complex_mixed {
    /* Basic fields */
    int id;
    
    /* Struct field */
    struct simple_struct data;
    
    /* Union field */
    union my_union variant;
    
    /* Callback fields */
    simple_callback_fn simple_handler;
    process_callback_fn complex_handler;
    finalize_callback_fn cleanup_handler;
    
    /* Pointer to callback */
    simple_callback_fn* GTY((skip)) handler_ptr;
    
    /* Array with callback elements */
    process_callback_fn processors[3];
    
    /* Nested struct with callback */
    struct callback_container container;
};

/* Language-specific structure (simulated) */
struct GTY(()) lang_specific {
    int lang_id;
    simple_callback_fn lang_handler;
};

/* String type example */
struct GTY(()) string_container {
    const char* GTY((length("strlen(value)+1"))) value;
    simple_callback_fn string_callback;
};

/* Self-referential structure with callback */
struct GTY(()) recursive_callback {
    int value;
    simple_callback_fn handler;
    struct recursive_callback* GTY((tag("RECURSIVE_PTR"))) next;
};

/* Multiple callback typedefs for comprehensive coverage */
typedef void (*start_callback_fn)(void) GTY((callback));
typedef int (*validate_callback_fn)(const void*) GTY((callback));
typedef void (*error_callback_fn)(int, const char*) GTY((callback));

/* Struct using multiple callback typedefs */
struct GTY(()) multi_callback {
    start_callback_fn start;
    validate_callback_fn validate;
    error_callback_fn error;
    simple_callback_fn simple;
};

#endif /* GTY_CALLBACK_TEST_H */
