#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar types (TYPE_SCALAR) */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* String type (TYPE_STRING) */
typedef const char * GTY(()) string_t;

/* Struct types (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct * GTY((skip)) nested;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct GTY(()) user_def_struct {
    int data;
    void * GTY((skip)) opaque;
} user_struct_t;

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback/function pointer type (TYPE_CALLBACK) */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) comparator_t)(const void *, const void *);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct GTY((user)) lang_specific_struct {
    int lang_data;
    callback_t handler;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_ptr_t structs[5];
    union_array_t variants;
    callback_t callbacks[3];
    struct lang_specific_struct * GTY((skip)) lang_struct;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
