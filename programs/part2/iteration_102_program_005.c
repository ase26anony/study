/* gty-callback-test.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 3. Plain struct (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    int field;
    double value;
};

/* 4. Union type (TYPE_UNION) */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;  /* skip annotation for pointer */
    double c;
};

/* 5. Pointer type (TYPE_POINTER) */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 6. Array type within a struct (TYPE_ARRAY) */
struct GTY(()) with_array {
    int arr[10];
    char name[32];
};

/* 7. Scalar typedef (TYPE_SCALAR) */
typedef unsigned long my_scalar GTY((length));

/* 8. String type (TYPE_STRING) - char* with special handling */
typedef const char* my_string GTY((length));

/* 9. Struct containing callback pointers (nested callback) */
struct GTY(()) callback_container {
    /* Direct callback pointer */
    simple_callback_fn handler GTY((skip));
    
    /* Array of callbacks */
    complex_callback_fn callbacks[3];
    
    /* Union containing callback */
    union {
        simple_callback_fn fn;
        int id;
    } GTY((desc("1"))) callback_union;
};

/* 10. Another struct with multiple callback types */
struct GTY(()) multi_callback_struct {
    /* Multiple callback fields */
    simple_callback_fn start_fn;
    complex_callback_fn process_fn;
    simple_callback_fn end_fn;
    
    /* Regular fields */
    int state;
    struct_ptr next;
};

/* 11. Union with callback as one variant */
union GTY(()) callback_or_data {
    simple_callback_fn callback;
    int data;
    char* str;
};

/* 12. Typedef struct with callback (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
    simple_callback_fn notify;
    int priority;
} callback_wrapper;

/* 13. Struct with callback pointer array */
struct GTY(()) callback_array_container {
    /* Array of callback pointers */
    simple_callback_fn handlers[5];
    
    /* Dynamic array of callbacks */
    complex_callback_fn* dynamic_handlers GTY((length("dynamic_count")));
    unsigned dynamic_count;
};

/* 14. Nested struct with callback */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        simple_callback_fn inner_callback;
        int inner_value;
    } inner_struct;
    
    complex_callback_fn outer_callback;
};

/* 15. Callback in a chain structure */
struct GTY(()) callback_chain {
    simple_callback_fn current;
    struct callback_chain* GTY((skip)) next;
};

#endif /* GTY_CALLBACK_TEST_H */
