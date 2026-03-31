#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;

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
typedef char* string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    struct undefined_struct* forward_ptr;  /* Forward reference */
    volatile int counter;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Padding */
    signed int value : 20;
};

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
            int int_val;
            float float_val;
            char* str_val;
        } data;
    } value;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, char*);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_5x5[5][5];
typedef float float_array_3d[3][3][3];
typedef void_func_ptr func_ptr_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*callback_int)(int);
typedef void (*callback_void)(void*, va_list);
typedef char* (*callback_string)(int, ...);

/* Complex nested type */
struct master_struct {
    /* All basic types */
    scalar_int s_int;
    scalar_float s_float;
    
    /* Pointer types */
    int_ptr ptr_to_int;
    struct master_struct* self_ptr;
    struct master_struct** double_self_ptr;
    
    /* Array types */
    int_array_10 ints;
    char_array_5x5 chars;
    
    /* Union */
    union simple_union data;
    
    /* Function pointer */
    callback_int processor;
    
    /* Nested anonymous struct */
    struct {
        int hidden;
        char secret;
    } __attribute__((aligned(16)));
    
    /* Flexible array member */
    int flexible_array[];
};

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Opaque handle type */
typedef struct undefined_struct* opaque_handle;

#endif /* TYPE_DEFS_H */
