/* Test header for gengtype coverage - contains all type categories */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for pointer types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
    my_scalar_t field1;
    int GTY(()) field2;
    const char * GTY(()) name;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
    int data;
    void (*GTY((skip)) mark_func)(void *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
    int int_val;
    my_scalar_t scalar_val;
    struct my_test_struct * GTY((skip)) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((chain_next)) union_ptr_chain;
typedef struct my_test_struct * GTY(()) struct_ptr_t;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
extern struct my_test_struct GTY(()) struct_array[5];
typedef int GTY(()) matrix_t[3][3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func_t)(int, const char *);
extern callback_func_t GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
    int lang_specific;
    tree GTY((tag("0"))) chain;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_type {
    /* Contains multiple type categories */
    my_scalar_t scalar_field;          /* TYPE_SCALAR */
    const char * GTY(()) string_field; /* TYPE_STRING */
    struct my_test_struct * GTY(()) ptr_field; /* TYPE_POINTER */
    int GTY(()) array_field[5];        /* TYPE_ARRAY */
    union my_test_union union_field;   /* TYPE_UNION */
    callback_func_t callback_field;    /* TYPE_CALLBACK */
    
    /* Nested struct */
    struct GTY(()) nested {
        int depth;
        struct complex_type * GTY((skip)) parent;
    } nested_field;
};

/* Variable-length array with length specifier */
struct GTY(()) varray_struct {
    int count;
    int GTY((length ("%h.count"))) data[1];
};

/* Pointer with special handling */
typedef struct my_test_struct * GTY((reorder ("test_reorder"))) reorder_ptr_t;

#endif /* MYTEST_H */
