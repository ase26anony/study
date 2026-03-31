#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>
#include <complex.h>

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
typedef long double my_longdouble;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String types (TYPE_STRING) */
typedef const char* my_string;
typedef char* mutable_string;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct *next;
    int data;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
} __attribute__((packed));

struct array_member_struct {
    int ids[10];
    float matrix[3][3];
    char name[50];
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union tagged_union {
    struct {
        int type;
    } header;
    struct {
        int type;
        int value;
    } int_data;
    struct {
        int type;
        double value;
    } double_data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*binary_op_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef float matrix_3x3[3][3];
typedef char*** complex_array[5][5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*error_handler_t)(int, const char*, ...);
typedef void (*va_func_t)(int, va_list);
typedef struct simple_struct* (*factory_t)(int);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
            int segments;
        } circle;
    } shape;
};

/* Opaque handle */
typedef struct opaque_handle* handle_t;

/* Function declarations */
void use_all_types(void);

#endif /* TYPES_H */
