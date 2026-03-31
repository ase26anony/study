#ifndef CALLBACK_TEST_1_H
#define CALLBACK_TEST_1_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with GTY((callback)) */
typedef void (*callback_func)(int) GTY((callback));

/* TYPE_STRUCT: Simple struct with GTY(()) */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* TYPE_POINTER: Pointer type with GTY tag */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_CALLBACK inside TYPE_STRUCT: Struct containing callback */
struct GTY(()) struct_with_callback {
    callback_func handler;
    struct_ptr next;
};

/* TYPE_ARRAY: Array within struct */
struct GTY(()) struct_with_array {
    int values[10];
    callback_func callbacks[5];
};

/* TYPE_UNION: Union with GTY(()) */
union GTY(()) callback_union {
    callback_func fn;
    int id;
    void* data;
};

#endif /* CALLBACK_TEST_1_H */
