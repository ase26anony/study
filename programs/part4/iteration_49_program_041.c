#ifndef TYPES_H
#define TYPES_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct undefined_struct;      /* Forward declaration */
union undefined_union;        /* Forward declaration */

/* Scalar types (TYPE_SCALAR) */
typedef char scalar_char;
typedef short scalar_short;
typedef int scalar_int;
typedef long scalar_long;
typedef long long scalar_long_long;
typedef float scalar_float;
typedef double scalar_double;
typedef long double scalar_long_double;
typedef _Bool scalar_bool;
typedef _Complex float scalar_complex_float;
typedef _Complex double scalar_complex_double;
typedef __int128 scalar_int128;  /* GNU extension */

/* String types (TYPE_STRING) */
typedef char* string_ptr;
typedef const char* const_string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct* next;  /* Self-referential */
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
} __attribute__((packed));

struct array_member_struct {
    int numbers[10];
    char name[50];
    struct simple_struct structs[5];
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
            int int_value;
            float float_value;
            char* string_value;
        };
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][10];
typedef int int_array_3d[3][4][5];
typedef struct simple_struct struct_array[8];

/* Callback types (TYPE_CALLBACK) */
typedef int (*int_callback)(int);
typedef void (*void_callback)(void);
typedef char* (*string_callback)(int, const char*);
typedef int (*variadic_callback)(int, ...);

/* Language-specific struct (TYPE_LANG_STRUCT) - using va_list */
typedef struct {
    va_list args;
    int count;
} lang_struct_wrapper;

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char name[32];
    void* data;
} user_struct_t;

/* Vector types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int as_int;
        float as_float;
    } value;
};

/* Function pointer with complex signature */
typedef int (*complex_callback)(
    struct simple_struct*,
    union simple_union*,
    int_array_2d,
    void (*)(void)
);

/* Opaque pointer type chain */
typedef struct undefined_struct* opaque_ptr;
typedef opaque_ptr* opaque_ptr_ptr;

#endif /* TYPES_H */
