#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_t;

/* Simple struct - TYPE_STRUCT */
struct GTY(()) simple_struct {
    scalar_int_t id;
    scalar_float_t value;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) user_struct {
    scalar_int_t data;
    string_t name;
} user_struct_t;

/* Union - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t int_val;
    scalar_float_t float_val;
    string_t str_val;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) process_func_t)(const char *, int);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_struct {
    int lang_specific_data;
    callback_t lang_callback;
};

/* Complex nested structure to ensure traversal */
struct GTY(()) complex_nested {
    /* Contains scalar */
    scalar_int_t count;
    
    /* Contains string */
    string_t description;
    
    /* Contains struct */
    struct simple_struct simple;
    
    /* Contains user struct */
    user_struct_t user;
    
    /* Contains union */
    union data_union data;
    
    /* Contains pointer */
    int_ptr_t ptr;
    
    /* Contains array */
    int_array_t numbers;
    
    /* Contains callback */
    callback_t handler;
    
    /* Contains language struct */
    struct lang_struct lang_data;
    
    /* Nested pointer to undefined struct */
    struct undefined_struct * GTY(()) undefined_ptr;
};

/* Another struct with array of pointers */
struct GTY(()) pointer_container {
    struct_ptr_t struct_ptrs[4];
    union_ptr_t union_ptrs[2];
    callback_t callbacks[3];
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
