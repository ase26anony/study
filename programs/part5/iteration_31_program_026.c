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
typedef long GTY(()) scalar_long_t;

/* String type (TYPE_STRING) */
typedef const char * GTY(()) string_t;

/* Struct types (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    int a;
    float b;
};

typedef struct GTY(()) named_struct {
    scalar_int_t id;
    string_t name;
} named_struct_t;

/* User struct (TYPE_USER_STRUCT) */
/* This is created when we have a typedef of a struct */
typedef struct GTY(()) user_struct_def {
    int x;
    double y;
} user_struct_t;

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char * GTY((skip)) string_val;
};

/* Pointer types (TYPE_POINTER) */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef int_ptr_t GTY(()) ptr_array_t[8];

/* Callback types (TYPE_CALLBACK) */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) compare_func_t)(const void*, const void*);

/* Language-specific struct (TYPE_LANG_STRUCT) */
/* These are marked with special GTY options */
struct GTY((desc("%0.tag"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int tag;
    struct lang_struct *next;
    struct lang_struct *prev;
    union data_union data;
};

/* Complex nested types to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t scalar_field;
    string_t string_field;
    struct simple_struct struct_field;
    union data_union union_field;
    int_ptr_t pointer_field;
    int_array_t array_field;
    callback_t callback_field;
    struct lang_struct * GTY(()) lang_struct_field;
    
    /* Nested array of pointers */
    struct simple_struct * GTY(()) ptr_array[4];
    
    /* Pointer to array */
    int (* GTY(()) matrix_ptr)[3][3];
};

/* Another undefined type */
union undefined_union;

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
