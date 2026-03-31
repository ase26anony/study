#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

/* Scalar types (TYPE_SCALAR) */
typedef char char_type;
typedef short short_type;
typedef int int_type;
typedef long long_type;
typedef long long long_long_type;
typedef float float_type;
typedef double double_type;
typedef _Bool bool_type;
typedef _Complex float complex_float_type;
typedef _Complex double complex_double_type;
typedef __int128 int128_type __attribute__((aligned(16)));

/* String types (TYPE_STRING) */
typedef char* string_type;
typedef const char* const_string_type;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct complex_struct {
    int id;
    char name[32];
    struct simple_struct nested;
    void* data;
};

struct packed_struct {
    char a;
    int b;
    short c;
} __attribute__((packed));

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

/* Anonymous struct/union */
struct container {
    int type;
    union {
        int int_value;
        float float_value;
        char* string_value;
    } data;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char c;
    void* p;
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
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
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[20];
typedef void (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int result);
typedef char* (*string_transform)(const char*, va_list);

/* Language-specific struct (TYPE_LANG_STRUCT) */
typedef __builtin_va_list va_list_type;

/* Vector types */
typedef int v4si __attribute__ ((vector_size (16)));

/* Opaque handle */
typedef struct {
    void* internal;
    int version;
} opaque_handle;

/* Linked list structure */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree structure */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

/* Function declarations */
void use_all_types(void);
extern void external_function(void*);

#endif /* TYPE_DEFS_H */
