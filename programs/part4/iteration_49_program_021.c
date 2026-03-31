#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <complex.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct undefined_struct;  /* Forward declaration */
union undefined_union;    /* Forward declaration */

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
typedef __int128 int128_t;  /* GNU extension */

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Callback types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef double (*math_callback)(double, double);

/* Pointer types (TYPE_POINTER) */
typedef void* void_ptr;
typedef int*** triple_int_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct simple_struct user_struct_t;

/* Union types (TYPE_UNION) */
union data_union {
    int i;
    float f;
    char* str;
    void* ptr;
};

/* Complex nested types */
struct complex_node {
    int data;
    struct complex_node* next;  /* Pointer to same type */
    struct complex_node* prev;
    union data_union value;
    void (*print)(struct complex_node*);
};

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } data;
};

/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

/* Vector type (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));

/* Language struct placeholder */
struct lang_struct {
    void* lang_specific;
};

/* Function declarations */
void use_all_types(void);
extern void external_func(void*);

#endif /* TYPE_DEFS_H */
