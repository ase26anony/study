#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Never defined */
union forward_declared_union;    /* Never defined */

/* Scalar types (TYPE_SCALAR) */
typedef int my_int;
typedef char my_char;
typedef short my_short;
typedef long my_long;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
};

/* Packed struct with bitfields */
struct packed_struct {
    int a : 3;
    int b : 5;
    int c : 10;
    int d : 14;
} __attribute__((packed));

/* Aligned struct */
struct aligned_struct {
    double data[4];
} __attribute__((aligned(64)));

/* Nested struct */
struct outer_struct {
    int id;
    struct simple_struct inner;
    struct {
        int anonymous_member;
        float another;
    } anonymous;
};

/* Self-referential struct (linked list) */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree node */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

/* Tagged union */
struct tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
};

/* Anonymous union within struct */
struct with_anonymous_union {
    int type;
    union {
        int num;
        double dbl;
        char* str;
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef struct simple_struct** struct_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][5];
typedef int int_array_3d[3][3][3];
typedef struct simple_struct struct_array[5];
typedef union simple_union union_array[8];
typedef void_func_ptr func_ptr_array[4];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int result);
typedef char* (*string_transform)(const char*, int);
typedef void (*varargs_func)(int, ...);
typedef int (*complex_callback)(struct tree_node*, union simple_union, ...);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Function pointer with attributes */
typedef void (*noreturn_func)(void) __attribute__((noreturn));
typedef int (*pure_func)(int) __attribute__((pure));
typedef int (*const_func)(int) __attribute__((const));

/* Opaque handle */
typedef struct opaque_handle* handle_t;

/* Complex type with all features */
struct mega_struct {
    /* Scalar members */
    int scalar_int;
    volatile long volatile_long;
    const double const_double;
    
    /* Pointer members */
    struct mega_struct* self_ptr;
    struct mega_struct** double_ptr;
    int (*func_ptr_member)(struct mega_struct*, int);
    
    /* Array members */
    int matrix[4][4];
    char buffer[256];
    void* ptr_array[8];
    
    /* Union member */
    union {
        struct {
            int x, y;
        } point;
        struct {
            float r, g, b, a;
        } color;
    } variant;
    
    /* Nested struct */
    struct {
        int depth;
        struct tree_node* root;
    } tree_info;
    
    /* Bitfield */
    unsigned int flags : 8;
    unsigned int state : 4;
    
    /* Flexible array member */
    int flexible_array[];
} __attribute__((aligned(32)));

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */
struct lang_struct_marker {
    int gcc_internal;
};

/* Function declarations */
void use_all_types(void);
void take_struct_ptr(struct simple_struct* s);
void take_union_ptr(union simple_union* u);
void take_func_ptr(binary_op op);
void take_varargs(varargs_func func, ...);

/* External references to prevent optimization */
extern volatile int external_counter;

#endif /* TYPE_DEFS_H */
