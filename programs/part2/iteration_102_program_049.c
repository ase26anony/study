/* gty-test-callback.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_TEST_CALLBACK_H
#define GTY_TEST_CALLBACK_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* 1. Define a callback function pointer type with GTY((callback)) */
typedef void (*simple_callback)(int) GTY((callback));

/* 2. Another callback type with parameters */
typedef int (*complex_callback)(const char*, void*) GTY((callback));

/* 3. Plain struct to trigger TYPE_STRUCT case */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* 4. Union to trigger TYPE_UNION case */
union GTY(()) my_union {
    int a;
    void* GTY((skip)) b;  /* skip annotation for pointer */
    double c;
};

/* 5. Pointer type to trigger TYPE_POINTER case */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 6. Array type within a struct to trigger TYPE_ARRAY case */
struct GTY(()) with_array {
    int arr[10];
    char* GTY((length)) str;
};

/* 7. Scalar typedef to trigger TYPE_SCALAR case */
typedef unsigned long my_scalar GTY((length));

/* 8. Struct containing callback pointer - nested callback */
struct GTY(()) callback_container {
    simple_callback handler;
    complex_callback processor;
    int id;
};

/* 9. Array of callbacks */
struct GTY(()) multi_handler {
    simple_callback handlers[5];
    int count;
};

/* 10. Union containing callback */
union GTY(()) callback_union {
    simple_callback fn;
    int callback_id;
    void* data;
};

/* 11. Struct with callback in nested struct */
struct GTY(()) outer_struct {
    struct GTY(()) inner {
        simple_callback cb;
        int value;
    } inner_data;
    callback_container* GTY((skip)) next;
};

/* 12. Typedef for callback returning callback */
typedef simple_callback (*callback_factory)(int) GTY((callback));

/* 13. Complex nested structure with multiple callback types */
struct GTY(()) complex_nested {
    callback_container main_handler;
    multi_handler backup_handlers;
    callback_union fallback;
    struct_ptr optional_data;
};

#endif /* GTY_TEST_CALLBACK_H */
