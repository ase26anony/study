#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;
typedef struct forward_declared_struct *fwd_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char char_type;
typedef short short_type;
typedef int int_type;
typedef long long_type;
typedef float float_type;
typedef double double_type;
typedef _Bool bool_type;
typedef _Complex float complex_float;
typedef _Complex double complex_double;
typedef __int128 int128_type;

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
    struct forward_declared_struct *fwd_ptr;
    void (*callback)(int);
} __attribute__((packed, aligned(8)));

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int x, y;
} point_t;

typedef struct tagged_struct {
    enum { TAG_INT, TAG_FLOAT, TAG_STRING } tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } value;
} tagged_union_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char c;
    void* p;
};

union nested_union {
    struct {
        int type;
        union simple_union data;
    } header;
    long long big_value;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, char*);
typedef struct complex_struct* complex_struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef void* ptr_array[20];
typedef int (*func_ptr_array[5])(void);

/* Callback types (TYPE_CALLBACK) */
typedef void (*simple_callback)(void);
typedef int (*complex_callback)(int, char*, ...);
typedef void (*struct_callback)(struct complex_struct*);
typedef union simple_union* (*union_generator)(int);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list va_list_type;
typedef __SIZE_TYPE__ size_type;

/* Vector types */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* Function pointer with attributes */
typedef void (*noreturn_func)(void) __attribute__((noreturn));

/* Opaque handle */
typedef void* opaque_handle;

/* Linked list structure */
struct list_node {
    int data;
    struct list_node* next;
    struct list_node* prev;
};

/* Tree structure */
struct tree_node {
    int key;
    struct tree_node* left;
    struct tree_node* right;
    void* data;
};

/* Function declarations */
void use_all_types(void);
extern void external_function(opaque_handle);

#endif /* TYPE_DEFS_H */
