#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct* undefined_ptr_t;

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
typedef __int128 int128_type;
typedef unsigned __int128 uint128_type;

/* String type (TYPE_STRING) */
typedef char* string_type;
typedef const char* const_string_type;

/* Function pointer types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef int (*math_callback)(double, double);
typedef void* (*alloc_callback)(size_t);
typedef int (*comparator)(const void*, const void*);

/* Vector types */
typedef int __attribute__((vector_size(16))) v4si;
typedef float __attribute__((vector_size(16))) v4sf;

/* Packed struct */
struct __attribute__((packed)) packed_struct {
    char c;
    int i;
    short s;
};

/* Aligned struct */
struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    long counter;
};

/* Main complex types to be defined in type_defs.c */
struct complex_struct;
union tagged_union;
typedef struct node linked_list_node;

/* Opaque handle */
typedef struct opaque* handle_t;

#endif /* TYPE_DEFS_H */
