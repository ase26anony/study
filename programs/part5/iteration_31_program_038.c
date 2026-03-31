#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED case */
struct undefined_struct;

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_t;

/* Struct types - TYPE_STRUCT */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    string_t name;
    struct simple_struct * GTY((skip)) nested;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct simple_struct GTY(()) user_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct complex_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) filter_func_t)(const char *, int);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_tag;
    void * GTY((skip)) lang_data;
    callback_t lang_callback;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_ptr_t structs;
    union_array_t variants;
    callback_t handler;
    struct GTY((skip)) {
        int hidden_field;
    } hidden;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
