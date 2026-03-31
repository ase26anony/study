#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Will never be defined */
union forward_declared_union;    /* Will never be defined */

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
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String type (TYPE_STRING) */
typedef char* string_ptr_t;
typedef const char* const_string_ptr_t;

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct __attribute__((packed)) packed_struct {
    int x;
    char y;
    short z;
};

struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    int tag;
};

struct complex_struct {
    struct simple_struct nested;
    struct packed_struct* packed_ptr;
    int array_member[10];
    volatile int volatile_member;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

union __attribute__((packed)) packed_union {
    long long as_ll;
    double as_double;
    char bytes[8];
};

/* Anonymous struct/union */
struct container {
    int type;
    union {
        struct simple_struct s;
        union simple_union u;
        double d;
    } data;
    struct {
        int x, y, z;
    } coordinates;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef struct complex_struct** struct_dbl_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef int (*int_func_ptr_t)(int, char*);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef int int_2d_array_t[5][10];
typedef int int_3d_array_t[3][4][5];
typedef struct simple_struct struct_array_t[20];
typedef void (*func_ptr_array_t[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(int, void*);
typedef char* (*string_processor_t)(const char*, int);
typedef int (*variadic_func_t)(int, ...);
typedef void (*va_list_func_t)(va_list);

/* Language-specific struct (TYPE_LANG_STRUCT) */
/* This is typically for GCC internal types, but we can approximate */
typedef __builtin_va_list builtin_va_list_t;

/* Vector types (GNU extension) */
typedef int __attribute__((vector_size(16))) int_vec4_t;
typedef float __attribute__((vector_size(32))) float_vec8_t;

/* Bit-field struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int flag4 : 8;
    signed int value : 16;
};

/* Self-referential struct (linked list) */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree node with multiple pointer types */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
    void (*print_func)(struct tree_node*);
};

/* Function declarations */
void use_all_types(void);
extern void external_function(void*);

#endif /* TYPE_DEFS_H */
