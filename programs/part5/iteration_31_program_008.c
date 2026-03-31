#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
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

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef float * GTY(()) float_ptr_t;
typedef struct undefined_struct * GTY(()) undefined_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef char GTY(()) char_array_t[256];
typedef int_ptr_t GTY(()) ptr_array_t[5];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) callback_ret_t)(const char *);
typedef void (* GTY(()) complex_callback_t)(int, float, callback_t);

/* Struct types - TYPE_STRUCT */
struct GTY(()) simple_struct {
    int a;
    float b;
    char c;
};

struct GTY(()) nested_struct {
    int_array_t array;
    int_ptr_t ptr;
    struct simple_struct simple;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
    int id;
    string_t name;
    callback_t handler;
} user_struct_t;

/* Union types - TYPE_UNION */
union GTY(()) data_union {
    int i;
    float f;
    char * GTY((skip)) str;
    void * GTY((skip)) ptr;
};

/* Complex nested struct with union */
struct GTY(()) complex_struct {
    union data_union data;
    int_array_t scores;
    callback_t on_update;
    struct undefined_struct *forward_ref;
};

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    void * GTY((skip)) lang_data;
};

/* Array of pointers to unions */
typedef union data_union * GTY(()) union_ptr_array_t[8];

/* Struct containing array of callbacks */
struct GTY(()) callback_container {
    callback_t handlers[4];
    int priority;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
