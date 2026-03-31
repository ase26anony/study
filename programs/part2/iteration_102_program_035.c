/* test-callback-types.h - Header with GTY-annotated types including callbacks */
#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"
#include "coretypes.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 2. Another callback type with parameters */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* 3. Plain struct (TYPE_STRUCT case) */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* 4. Union (TYPE_UNION case) */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;  /* skip annotation for void* */
    double c;
};

/* 5. Pointer type (TYPE_POINTER case) */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 6. Array type within a struct (TYPE_ARRAY case) */
struct GTY(()) with_array {
    int arr[10];
    char name[32];
};

/* 7. Scalar typedef (TYPE_SCALAR case) */
typedef unsigned long my_scalar GTY((length));

/* 8. String type (TYPE_STRING case) */
typedef const char* my_string GTY((length));

/* 9. Struct containing callback pointers (nested callback) */
struct GTY(()) callback_container {
    /* Direct callback pointer */
    simple_callback_fn handler;
    
    /* Array of callbacks */
    complex_callback_fn handlers[3];
    
    /* Union containing callback */
    union GTY(()) {
        simple_callback_fn fn;
        int id;
    } callback_union;
    
    /* Pointer to struct with callback */
    struct GTY(()) nested {
        simple_callback_fn nested_cb;
        int data;
    }* nested_ptr;
};

/* 10. Another struct with multiple callback types */
struct GTY(()) multi_callback_struct {
    simple_callback_fn start_fn;
    complex_callback_fn process_fn;
    void (*finalize_fn)(void) GTY((callback));  /* inline callback definition */
    
    /* Chain of callbacks */
    struct multi_callback_struct* GTY((skip)) next;
};

/* 11. Typedef struct (TYPE_USER_STRUCT case) */
typedef struct GTY(()) {
    simple_callback_fn cb;
    int priority;
} callback_wrapper;

/* 12. Union with callback as one variant */
union GTY(()) variant_container {
    simple_callback_fn callback;
    struct_ptr sp;
    my_scalar value;
};

/* 13. Struct with conditional callback */
#ifdef SPECIAL_FEATURE
struct GTY(()) conditional_callback {
    complex_callback_fn special_handler;
    int flags;
};
#endif

/* 14. Callback in array context */
struct GTY(()) callback_array_container {
    /* Array of callback pointers */
    simple_callback_fn callbacks[5];
    
    /* Flexible array member with callbacks */
    complex_callback_fn dynamic_callbacks[];
};

#endif /* TEST_CALLBACK_TYPES_H */
