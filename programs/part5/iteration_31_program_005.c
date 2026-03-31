#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration - TYPE_UNDEFINED */
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
typedef struct simple_struct * GTY(()) struct_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef char GTY(()) char_array_t[256];
typedef struct simple_struct GTY(()) struct_array_t[5];

/* Callback types - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) compare_func_t)(const void *, const void *);

/* Simple struct - TYPE_STRUCT */
struct GTY(()) simple_struct {
    int id;
    float value;
    char * GTY(()) name;
};

/* More complex struct with nested types */
struct GTY(()) complex_struct {
    int GTY(()) count;
    int_array_t GTY(()) numbers;
    struct_ptr_t GTY(()) next;
    callback_t GTY(()) handler;
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char * GTY(()) string_val;
    void * GTY(()) ptr_val;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct GTY(()) user_defined {
    int user_id;
    union data_union GTY(()) data;
    callback_t GTY(()) user_callback;
} user_struct_t;

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_tag;
    void * GTY((skip)) lang_data;
    struct lang_specific_struct * GTY(()) next;
};

/* Include auxiliary types */
#include "test_types_aux.h"

/* Struct containing array of pointers to unions */
struct GTY(()) container_struct {
    int GTY(()) size;
    union data_union * GTY(()) items[20];
    struct undefined_struct * GTY(()) future;  /* Forward reference */
};

/* Function pointer in struct */
struct GTY(()) processor {
    int (* GTY(()) process)(int, char **);
    void (* GTY(()) cleanup)(void);
};

#endif /* TEST_TYPES_H */
