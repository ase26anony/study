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
    struct simple_struct * GTY(()) nested;
    string_t name;
    scalar_double_t value;
};

/* User struct (TYPE_USER_STRUCT) - typedef'd struct */
typedef struct GTY(()) {
    scalar_int_t id;
    string_t description;
} user_struct_t;

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void * GTY(()) as_pointer;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/* Callback types (TYPE_CALLBACK) */
typedef void (* GTY(()) callback_t)(int, string_t);
typedef int (* GTY(()) filter_func_t)(const void * GTY(()));

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    callback_t handler;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains array of pointers */
    struct_ptr_t GTY(()) ptr_array[4];
    
    /* Contains union */
    union data_union data;
    
    /* Contains callback */
    callback_t notify;
    
    /* Contains nested struct */
    struct GTY(()) {
        int nested_id;
        string_t nested_name;
    } inner;
    
    /* Contains array */
    int_array_t numbers;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
