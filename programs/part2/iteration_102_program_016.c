/* test-callback-types.h - Header file with GTY-annotated types for gengtype coverage */
#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_SCALAR: Scalar type with GTY annotation */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer type to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_UNION: Union type containing various fields */
union GTY(()) my_union {
    int int_val;
    simple_ptr ptr_val;
    my_scalar scalar_val;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) with_array {
    int arr[10];
    simple_callback_fn callbacks[5];
};

/* TYPE_CALLBACK nested in struct: Callback inside a structure */
struct GTY(()) callback_container {
    /* Direct callback field */
    simple_callback_fn handler GTY((tag("HANDLER")));
    
    /* Array of callbacks */
    process_callback_fn processors[3];
    
    /* Union containing callback */
    union GTY(()) {
        simple_callback_fn fn;
        int id;
    } callback_union;
};

/* TYPE_USER_STRUCT: Forward declaration for user-defined struct handling */
struct GTY(()) user_defined;
typedef struct user_defined user_defined_t;

/* More complex nested structure with callbacks */
struct GTY(()) complex_structure {
    /* TYPE_STRUCT nested field */
    simple_struct base;
    
    /* TYPE_UNION field */
    my_union data;
    
    /* TYPE_ARRAY field */
    with_array arrays;
    
    /* TYPE_CALLBACK fields */
    process_callback_fn start_callback;
    process_callback_fn end_callback;
    
    /* TYPE_POINTER to callback */
    simple_callback_fn* callback_ptr;
    
    /* Nested callback container */
    callback_container container;
    
    /* Forward reference */
    user_defined_t* next;
};

/* Complete the user-defined struct */
struct GTY(()) user_defined {
    int id;
    complex_structure* complex;
    simple_callback_fn validate;
};

/* TYPE_LANG_STRUCT: Language-specific structure (simulated) */
struct GTY(()) lang_specific {
    int lang_id;
    void* GTY((skip)) lang_data;  /* Skip this field for GC */
    simple_callback_fn lang_callback;
};

/* TYPE_STRING: String type handling */
struct GTY(()) string_container {
    const char* GTY((length)) str;
    simple_callback_fn string_callback;
};

/* Union specifically for callbacks */
union GTY(()) callback_only_union {
    simple_callback_fn fn1;
    process_callback_fn fn2;
    void (*fn3)(void) GTY((callback));
};

/* Structure with multiple callback types */
struct GTY(()) multi_callback {
    /* Different callback signatures */
    void (*void_callback)(void) GTY((callback));
    int (*int_callback)(int, int) GTY((callback));
    char* (*string_callback)(const char*) GTY((callback));
    
    /* Pointer to callback (not itself a callback type) */
    simple_callback_fn* callback_array_ptr;
    
    /* Callback in array */
    process_callback_fn proc_array[4];
};

/* Callback with structure parameter */
struct GTY(()) data_buffer;
typedef void (*buffer_callback_fn)(struct data_buffer*) GTY((callback));

struct GTY(()) data_buffer {
    char* buffer;
    size_t size;
    buffer_callback_fn on_complete;
    buffer_callback_fn on_error;
};

/* Chain of structures with callbacks */
struct GTY(()) callback_chain {
    int chain_id;
    simple_callback_fn chain_callback;
    struct callback_chain* next;
};

/* Mixed types in array */
struct GTY(()) mixed_container {
    /* Array containing different types */
    union {
        int i;
        simple_callback_fn fn;
        simple_ptr ptr;
    } GTY((desc("0"))) items[20];
};

#endif /* TEST_CALLBACK_TYPES_H */
