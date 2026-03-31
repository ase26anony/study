#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long long GTY(()) scalar_ll_t;

/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    string_t name;
    struct simple_struct * GTY((skip)) nested;
};

/* TYPE_USER_STRUCT: User-defined struct (using typedef) */
typedef struct GTY(()) {
    int x;
    int y;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer types */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(())) compare_func_t)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific struct with GTY markers */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    callback_t handler;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t scalar_field;
    string_t string_field;
    struct simple_struct struct_field;
    union data_union union_field;
    int_ptr_t pointer_field;
    int_array_t array_field;
    callback_t callback_field;
    struct lang_specific_struct * GTY((skip)) lang_struct_field;
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
