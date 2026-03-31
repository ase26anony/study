#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

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

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int count : 8;
    unsigned int : 4;  /* Padding */
    unsigned int value : 16;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct tagged_struct {
    enum { TAG_A, TAG_B, TAG_C } tag;
    union {
        int as_int;
        double as_double;
        char* as_string;
    } value;
} tagged_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char* s;
};

union nested_union {
    struct {
        int type;
        union simple_union data;
    } tagged;
    long raw;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[3];
typedef void_func_ptr func_ptr_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_with_context)(void*, int);
typedef char* (*string_transform)(const char*, va_list);
typedef struct simple_struct* (*struct_factory)(int, ...);

/* Language struct (TYPE_LANG_STRUCT) - using GCC extensions */
struct vector_struct {
    int __attribute__((vector_size(16))) v4;
    float __attribute__((vector_size(32))) v8;
};

struct aligned_struct {
    int a;
    double b;
} __attribute__((aligned(64)));

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
    void (*visit)(struct tree_node*);
};

/* Function declarations */
void use_all_types(void);
extern void external_func(void*);

/* Prevent dead code elimination */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x))

#endif /* TYPE_DEFS_H */
