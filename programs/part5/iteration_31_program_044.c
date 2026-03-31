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

/* Basic struct (TYPE_STRUCT) */
struct GTY(()) basic_struct {
    scalar_int_t id;
    scalar_float_t value;
    string_t name;
};

/* User struct (TYPE_USER_STRUCT) */
typedef struct GTY(()) user_struct_def {
    int data;
    struct basic_struct *link;
} user_struct_t;

/* Union (TYPE_UNION) */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void *as_pointer;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct basic_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct basic_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback/function pointer (TYPE_CALLBACK) */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) compare_func_t)(const void*, const void*);

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct GTY((user)) lang_specific_struct {
    int lang_data;
    callback_t lang_callback;
    struct undefined_struct *forward_ref;  /* Uses undefined type */
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
    /* Contains all type categories */
    scalar_int_t scalar_field;
    string_t string_field;
    struct basic_struct struct_field;
    user_struct_t user_struct_field;
    union data_union union_field;
    int_ptr_t pointer_field;
    int_array_t array_field;
    callback_t callback_field;
    struct lang_specific_struct *lang_struct_field;
    
    /* Nested arrays of pointers */
    struct basic_struct * GTY(()) struct_ptr_array[4];
    union data_union * GTY(()) union_ptr_array[2];
    
    /* Pointer to array */
    int (* GTY(()) array_ptr)[10];
    
    /* Callback that returns pointer */
    union data_union * (* GTY(()) get_data)(int);
};

/* Another union with struct members */
union GTY(()) complex_union {
    struct container_struct as_container;
    struct lang_specific_struct *as_lang_ptr;
    callback_t as_callback;
    int_array_t as_array;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
