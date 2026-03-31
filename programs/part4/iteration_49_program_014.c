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
    struct nested_struct* next;  /* Linked list */
    int data;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 24;  /* Padding */
} __attribute__((packed));

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
            int i;
            float f;
            char* s;
        } value;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][5];
typedef int int_array_3d[3][3][3];
typedef char* string_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int result);
typedef int (*variadic_func)(int, ...);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Anonymous struct/union */
struct container {
    int id;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
    } shape;
};

/* Language-specific struct (TYPE_LANG_STRUCT simulation) */
struct lang_struct {
    void* lang_specific;
    int lang_tag;
} __attribute__((aligned(16)));

/* Function declarations */
void use_all_types(void);
extern void external_func(void*);

#endif /* TYPE_DEFS_H */
