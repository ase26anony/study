#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
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

/* Struct types (TYPE_STRUCT, TYPE_USER_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct complex_struct {
    int id;
    char name[32];
    struct simple_struct nested;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
    volatile int volatile_member;
} __attribute__((aligned(16)));

/* Anonymous struct/union */
struct with_anonymous {
    union {
        int as_int;
        float as_float;
    };
    struct {
        short x;
        short y;
    } point;
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Padding */
    signed int value : 16;
};

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char c;
    void* p;
};

union tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    struct {
        int tag;
        union {
            int int_val;
            float float_val;
            char* string_val;
        };
    };
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*void_func_ptr)(void);
typedef int* const const_int_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef struct simple_struct struct_array[20];
typedef int (*func_ptr_array[5])(void);

/* Callback/Function pointer types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef struct simple_struct* (*struct_callback)(int, float);
typedef union simple_union (*union_callback)(int, int);
typedef int (*callback_returning_array_ptr)[10];
typedef void (*nested_callback)(int (*)(float), char*);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language-specific struct placeholder (TYPE_LANG_STRUCT) */
struct lang_struct_placeholder {
    void* lang_specific;
};

/* Variadic types */
typedef va_list va_list_type;

/* Opaque function declarations */
void use_all_types(void*);
void force_type_usage(void);

#endif /* TYPE_DEFS_H */
