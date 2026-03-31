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
typedef struct simple_struct GTY(()) user_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char * GTY((skip)) string_val;
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
typedef int (* GTY(()) compare_func_t)(const void *, const void *);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_struct_base {
    int lang_specific_field;
};

struct GTY((user, desc("%0"))) lang_struct_derived {
    struct lang_struct_base base;
    void * GTY((skip)) extra_data;
};

/* Nested complex type combining multiple categories */
struct GTY(()) container_struct {
    scalar_int_t count;                     /* TYPE_SCALAR */
    string_t description;                   /* TYPE_STRING */
    struct complex_struct items[5];         /* TYPE_ARRAY of TYPE_STRUCT */
    union data_union * GTY((skip)) variants; /* TYPE_POINTER to TYPE_UNION */
    callback_t handler;                     /* TYPE_CALLBACK */
    struct undefined_struct * GTY((skip)) future; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
