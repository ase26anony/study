#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

/* Scalar types (TYPE_SCALAR) */
typedef int scalar_int;
typedef char scalar_char;
typedef short scalar_short;
typedef long scalar_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* String type (TYPE_STRING) */
typedef const char* string_type;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

struct nested_struct {
    struct simple_struct inner;
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

struct packed_struct {
    char a;
    int b;
    short c;
} __attribute__((packed));

struct aligned_struct {
    double d;
    int i;
} __attribute__((aligned(64)));

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            const char* string_value;
        };
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_matrix_3x3[3][3];
typedef int int_cube_2x2x2[2][2][2];
typedef const char* string_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_with_context)(void* context, int data);
typedef int (*variadic_func)(int, ...);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */
typedef struct __builtin_va_list va_list_type;

/* Complex type relationships */
struct node {
    int data;
    struct node* next;
    struct node* prev;
};

struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    union {
        void* user_data;
        int metadata;
    } info;
};

/* Function pointer arrays */
typedef int (*op_funcs[4])(int, int);

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int id;
        char name[4];
    } identifier;
};

/* External declarations */
extern struct forward_declared_struct* external_forward_ptr;
extern void use_all_types(void*);

#endif /* TYPE_DEFS_H */
