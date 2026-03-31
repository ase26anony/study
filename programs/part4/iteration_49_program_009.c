#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char my_char;
typedef short my_short;
typedef int my_int;
typedef long my_long;
typedef long long my_longlong;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String type (TYPE_STRING) */
typedef const char* my_string_t;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    double z;
} __attribute__((aligned(16)));

struct complex_struct;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct complex_struct* struct_ptr_t;
typedef void (*generic_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef int int_matrix_3x3_t[3][3];
typedef int int_cube_2x2x2_t[2][2][2];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_no_args_t)(void);
typedef char* (*string_transform_t)(const char*, int);
typedef void (*varargs_callback_t)(int, ...);

/* Language-specific struct (TYPE_LANG_STRUCT simulation) */
struct __attribute__((transparent_union)) transparent_union {
    int i;
    float f;
};

/* Complex nested type definitions */
struct complex_struct {
    /* Scalar members */
    int id;
    double value;
    
    /* Pointer member */
    struct complex_struct* next;
    struct complex_struct* prev;
    
    /* Array member */
    int data[8];
    
    /* Union member */
    union {
        int tag;
        void* ptr;
    } variant;
    
    /* Nested struct */
    struct {
        int x, y;
    } position;
    
    /* Bit-field */
    unsigned int flags : 4;
    unsigned int state : 2;
    
    /* Function pointer */
    binary_op_t operation;
    
    /* Flexible array member */
    int flexible_array[];
} __attribute__((packed));

/* Tree node with recursive definition */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Linked list */
struct list_node {
    void* data;
    struct list_node* next;
    struct list_node* (*allocator)(void);
};

/* Anonymous struct/union */
struct container {
    struct {
        int a, b;
    };  /* Anonymous struct */
    
    union {
        int x;
        double y;
    };  /* Anonymous union */
};

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function declarations */
void use_all_types(void);
void opaque_use(void* ptr);

#endif /* TYPES_H */
