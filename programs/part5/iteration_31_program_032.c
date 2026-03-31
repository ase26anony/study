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

/* Struct types - TYPE_STRUCT */
struct GTY(()) simple_struct {
    scalar_int_t id;
    string_t name;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_float_t value;
    string_t description;
    struct simple_struct * GTY((skip)) simple;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct simple_struct GTY(()) user_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t int_val;
    scalar_float_t float_val;
    string_t string_val;
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
typedef int (* GTY(()) filter_t)(const char *, int);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_struct {
    int GTY(()) lang_specific_field;
    callback_t GTY(()) handler;
    struct lang_struct * GTY(()) next;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    int GTY(()) id;
    int_array_t GTY(()) numbers;
    struct_ptr_t GTY(()) items[8];
    union_ptr_t GTY(()) data;
    callback_t GTY(()) processor;
    struct lang_struct * GTY(()) lang_data;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
