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

/* String types (TYPE_STRING) */
typedef char* string_ptr_t;
typedef const char* const_string_ptr_t;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct nested_struct {
    struct simple_struct inner;
    double extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

/* Packed struct with alignment */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Tagged union */
union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
        } data;
    } tagged;
    char raw_data[16];
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef char char_array_5x5_t[5][5];
typedef float float_array_2x3x4_t[2][3][4];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef char* (*string_transform_t)(const char*, int);

/* Complex nested type with all categories */
struct master_struct {
    /* Scalar members */
    int id;
    double value;
    
    /* String member */
    char* name;
    
    /* Struct member */
    struct nested_struct nested;
    
    /* Union member */
    union simple_union data;
    
    /* Pointer members */
    struct master_struct* next;
    struct master_struct** prev;
    
    /* Array members */
    int scores[5];
    float matrix[3][3];
    
    /* Callback member */
    binary_op_t operation;
    
    /* Anonymous union */
    union {
        int as_int;
        float as_float;
    } anonymous;
    
    /* Bitfield */
    unsigned int flags : 8;
};

/* Vector types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* 128-bit integer */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Variadic type */
typedef va_list va_list_t;

/* Function declarations using these types */
void process_struct(struct master_struct* s);
int compute_with_callback(binary_op_t op, int a, int b);
void handle_strings(string_ptr_t* strings, int count);

#endif /* TYPE_DEFS_H */
