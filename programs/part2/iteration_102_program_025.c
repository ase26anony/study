/* gty-callback-test.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK 1: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/* TYPE_SCALAR: Scalar type with GTY marker */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

/* TYPE_POINTER: Pointer to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_CALLBACK 2: Another callback with different signature */
typedef int (*complex_callback_fn)(const char*, simple_ptr) GTY((callback));

/* TYPE_UNION: Union containing various types */
union GTY(()) mixed_union {
    simple_callback_fn cb;
    simple_ptr sp;
    int id;
    char* GTY((string)) name;
};

/* TYPE_STRUCT with nested callback */
struct GTY(()) container_struct {
    /* Array of callbacks - TYPE_ARRAY of TYPE_CALLBACK */
    simple_callback_fn handlers[4];
    
    /* Single complex callback */
    complex_callback_fn processor;
    
    /* Regular pointer */
    simple_ptr next;
    
    /* Union member */
    union mixed_union data;
};

/* TYPE_CALLBACK 3: Callback returning callback */
typedef simple_callback_fn (*meta_callback_fn)(void) GTY((callback));

/* TYPE_STRUCT with multiple callback types */
struct GTY(()) callback_container {
    /* Direct callback field */
    meta_callback_fn getter;
    
    /* Array of different callbacks */
    complex_callback_fn validators[2];
    
    /* Struct with callback */
    struct container_struct nested;
};

/* TYPE_UNION with callback alternative */
union GTY(()) callback_or_data {
    simple_callback_fn callback;
    struct container_struct* GTY((tag("CONTAINER_PTR"))) data;
    int value;
};

/* TYPE_ARRAY: Standalone array of callbacks */
typedef complex_callback_fn callback_array[3] GTY((tag("CALLBACK_ARRAY")));

/* TYPE_LANG_STRUCT: Simulating a language-specific structure */
struct GTY(()) lang_specific {
    int lang_code;
    void* GTY((skip)) lang_data;
    simple_callback_fn lang_callback;
};

/* TYPE_STRING: String type */
typedef const char* my_string GTY((string));

/* TYPE_STRUCT with string and callback */
struct GTY(()) string_and_callback {
    my_string name;
    simple_callback_fn notify;
    int id;
};

/* TYPE_CALLBACK 4: Callback using string type */
typedef void (*string_callback_fn)(my_string) GTY((callback));

/* TYPE_UNION containing all major types */
union GTY(()) mega_union {
    struct simple_struct ss;
    struct container_struct cs;
    simple_callback_fn cb1;
    complex_callback_fn cb2;
    string_callback_fn cb3;
    my_string str;
    int num;
};

#endif /* GTY_CALLBACK_TEST_H */
