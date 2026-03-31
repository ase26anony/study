#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration - will count as TYPE_UNDEFINED */
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
    scalar_int_t id;
    string_t name;
};

/* Another struct with nested types */
struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_float_t values[5];  /* Array member */
    struct basic_struct * GTY(()) next;  /* Pointer member */
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
    scalar_int_t data;
    scalar_char_t flag;
} user_struct_t;

/* Union - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
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
typedef int (* GTY(()) process_func_t)(const char *, int);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_tag;
    void * GTY((skip)) lang_data;
    callback_t lang_callback;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains all type categories */
    scalar_int_t scalar_field;           /* TYPE_SCALAR */
    string_t string_field;               /* TYPE_STRING */
    struct basic_struct struct_field;    /* TYPE_STRUCT */
    user_struct_t user_struct_field;     /* TYPE_USER_STRUCT */
    union data_union union_field;        /* TYPE_UNION */
    int_ptr_t pointer_field;             /* TYPE_POINTER */
    int_array_t array_field;             /* TYPE_ARRAY */
    callback_t callback_field;           /* TYPE_CALLBACK */
    struct lang_specific_struct * GTY(()) lang_field;  /* TYPE_LANG_STRUCT */
};

/* Function pointer with complex signature */
typedef void (* GTY(()) complex_callback_t)(
    struct container_struct *,
    int_array_t,
    callback_t
);

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
