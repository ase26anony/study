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
    int a;
    float b;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct * GTY((skip)) nested;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
    int x;
    double y;
} user_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char * GTY((skip)) str_val;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef int_ptr_t GTY(()) ptr_array_t[8];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int, const char *);
typedef int (* GTY(()) compare_func_t)(const void *, const void *);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_id;
    void * GTY((skip)) lang_data;
    callback_t lang_callback;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_ptr_t structs[4];
    union data_union variants[3];
    callback_t handlers[2];
    struct lang_specific_struct * GTY((skip)) lang_struct;
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
