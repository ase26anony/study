#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration - will be counted as TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_t;

/* Basic struct - TYPE_STRUCT */
struct GTY(()) basic_struct {
    scalar_int_t field1;
    scalar_float_t field2;
    string_t name;
};

/* Another struct with nested types */
struct GTY(()) complex_struct {
    struct basic_struct * GTY((skip)) nested;
    scalar_double_t value;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct basic_struct GTY(()) user_struct_t;

/* Union - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    struct basic_struct * GTY((skip)) as_struct;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct basic_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct basic_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(())) compare_func_t)(const void *, const void *);

/* Language-specific struct with GTY markers - TYPE_LANG_STRUCT */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    struct lang_struct * GTY((skip)) next;
    struct lang_struct * GTY((skip)) prev;
    string_t identifier;
    scalar_int_t kind;
    callback_t handler;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_array_t objects;
    union_array_t variants;
    struct_ptr_t * GTY((length("%0.count"))) pointers;
    int count;
    callback_t processors[3];
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
