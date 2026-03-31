#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK - Core callback type definition */
typedef void (*callback_fn_t)(int) GTY((callback));

/* TYPE_STRUCT - Simple struct type */
struct GTY(()) simple_struct {
    int field;
};

/* TYPE_UNION - Union type */
union GTY(()) simple_union {
    int a;
    void* GTY((skip)) b;  /* skip annotation for void* */
};

/* TYPE_POINTER - Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY - Array within struct */
struct GTY(()) array_container {
    int GTY((length("10"))) arr[10];
};

/* TYPE_SCALAR - Scalar typedef */
typedef unsigned int my_scalar GTY((length));

/* TYPE_STRING - String pointer */
typedef const char* my_string GTY((length));

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
    callback_fn_t handler;          /* TYPE_CALLBACK field */
    callback_fn_t GTY((skip)) opt_handler;  /* Optional callback */
};

/* Callback in union */
union GTY(()) callback_union {
    callback_fn_t fn;    /* TYPE_CALLBACK field */
    int id;
};

/* Array of callbacks */
struct GTY(()) callback_array {
    callback_fn_t GTY((length("3"))) handlers[3];
};

/* Complex nested structure with callback */
struct GTY(()) complex_struct {
    struct_ptr next;                /* TYPE_POINTER */
    simple_union data;              /* TYPE_UNION */
    callback_container cb_container; /* Contains TYPE_CALLBACK */
    my_string name;                 /* TYPE_STRING */
    my_scalar count;                /* TYPE_SCALAR */
};

#endif /* CALLBACK_TEST_H */
