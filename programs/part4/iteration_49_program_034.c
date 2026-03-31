#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <complex.h>

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

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct* next;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
} __attribute__((packed));

struct array_member_struct {
    int data[10];
    float matrix[3][3];
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
};

union tagged_union {
    int type;
    struct {
        int type;
        int data;
    } int_data;
    struct {
        int type;
        double data;
    } double_data;
};

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        struct { int x, y; } point;
        struct { float r, g, b; } color;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array[10];
typedef float float_2d_array[5][5];
typedef char* string_array[20];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_func)(void* data, int result);
typedef int (*variadic_func)(int count, ...);

/* Language-specific struct (TYPE_LANG_STRUCT simulation) */
struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
    void* metadata;
};

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Opaque handle */
typedef struct undefined_struct* opaque_handle;

/* Function declarations */
void use_all_types(void);

#endif /* TYPE_DEFS_H */
