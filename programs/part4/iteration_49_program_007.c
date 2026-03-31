#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Never defined */
union forward_declared_union;    /* Never defined */

/* Scalar types (TYPE_SCALAR) */
typedef int my_int_t;
typedef char my_char_t;
typedef short my_short_t;
typedef long my_long_t;
typedef float my_float_t;
typedef double my_double_t;
typedef _Bool my_bool_t;
typedef _Complex float my_complex_t;
typedef _Complex double my_dcomplex_t;

/* String type (TYPE_STRING) */
typedef char* string_t;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct complex_struct {
    int id;
    char name[32];
    struct simple_struct nested;
    struct complex_struct* next;  /* Linked list */
    void (*callback)(int);        /* Function pointer member */
};

/* Packed struct with bitfields */
struct packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int count : 12;
    char data;
} __attribute__((packed));

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Tagged union */
struct tagged_union {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct complex_struct* complex_struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef char char_matrix_5x5_t[5][5];
typedef float float_3d_array_t[3][3][3];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef char* (*string_transform_t)(const char*);
typedef void (*varargs_func_t)(int, ...);

/* Language struct (TYPE_LANG_STRUCT) - using va_list */
typedef struct {
    va_list args;
    int count;
} my_va_struct;

/* Vector types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* 128-bit integer */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int id;
        char tag[4];
    } identifier;
};

/* Function pointer with complex signature */
typedef struct complex_struct* (*factory_func_t)(int, const char*);

/* Recursive type definition */
typedef struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
} tree_node_t;

/* Array of function pointers */
typedef int (*op_funcs_t[4])(int, int);

/* Opaque handle */
typedef void* opaque_handle_t;

#endif /* TYPE_DEFS_H */
