/* test-callback-types.h - Header file with GTY-annotated types for gengtype coverage */

#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with parameters */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 3. Plain struct type */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* 4. Union type */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;  /* skip annotation for void* */
    simple_struct* c;
};

/* 5. Pointer type with tag */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* 6. Array type within a struct */
struct GTY(()) with_array {
    int arr[10];
    char* GTY((length("strlen($)"))) str;
};

/* 7. Scalar typedef with length attribute */
typedef unsigned my_scalar GTY((length));

/* 8. Struct containing callback function pointers */
struct GTY(()) callback_container {
    /* Direct callback field */
    simple_callback_fn handler GTY((skip));
    
    /* Array of callbacks */
    complex_callback_fn handlers[2];
    
    /* Pointer to callback */
    simple_callback_fn* callback_ptr;
};

/* 9. Union containing a callback */
union GTY(()) callback_union {
    simple_callback_fn fn;
    int id;
    simple_struct data;
};

/* 10. Nested struct with callback */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        simple_callback_fn cb;
        int value;
    } inner_data;
    
    callback_container container;
};

/* 11. Typedef struct with callback */
typedef struct GTY(()) {
    complex_callback_fn processor;
    my_scalar count;
} processor_t;

/* 12. Another callback type definition */
typedef void (*finalizer_fn)(void*) GTY((callback));

/* 13. Struct using multiple callback types */
struct GTY(()) multi_callback {
    simple_callback_fn start;
    complex_callback_fn process;
    finalizer_fn finish;
    processor_t* processor;
};

/* 14. Callback in array context */
struct GTY(()) callback_array_container {
    /* Array of callback pointers */
    simple_callback_fn callbacks[5];
    
    /* Variable length array of callbacks */
    complex_callback_fn* GTY((length("count"))) var_callbacks;
    int count;
};

/* 15. Language-specific structure (simulated) */
struct GTY(()) lang_specific {
    int lang_field;
    simple_callback_fn lang_callback;
};

#endif /* TEST_CALLBACK_TYPES_H */
