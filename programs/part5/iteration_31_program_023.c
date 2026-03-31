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
    scalar_int_t field1;
    scalar_float_t field2;
};

/* Another struct with different composition */
struct GTY(()) complex_struct {
    string_t name;
    scalar_double_t value;
    struct basic_struct *next;
};

/* User struct - TYPE_USER_STRUCT */
typedef struct basic_struct GTY(()) user_struct_t;

/* Union type - TYPE_UNION */
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

/* Callback type - TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) compare_func_t)(const void*, const void*);

/* Language-specific struct - TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    void * GTY((skip)) opaque_data;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains array of pointers */
    struct_ptr_t GTY(()) ptr_array[5];
    
    /* Contains union */
    union data_union GTY(()) data;
    
    /* Contains callback */
    callback_t GTY(()) handler;
    
    /* Contains string */
    string_t GTY(()) description;
    
    /* Nested struct */
    struct GTY(()) nested {
        int GTY(()) depth;
        string_t GTY(()) path;
    } inner;
};

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
