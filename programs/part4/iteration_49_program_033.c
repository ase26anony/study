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
    volatile int counter;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 4;  /* Padding */
    signed int value : 20;
} __attribute__((packed, aligned(8)));

/* Anonymous struct/union */
struct container {
    struct {
        int x;
        int y;
    } point;
    union {
        int as_int;
        float as_float;
    } data;
};

/* User struct (TYPE_USER_STRUCT) */
typedef struct simple_struct user_struct_t;
typedef struct nested_struct* nested_ptr_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char* s;
};

union tagged_union {
    enum { INT, FLOAT, STRING } tag;
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
        };
    } data;
} __attribute__((packed));

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
typedef void (*callback_func)(void* data, int result);
typedef int (*variadic_func)(int, ...);

/* Language struct (TYPE_LANG_STRUCT) */
typedef __builtin_va_list va_list_type;
typedef __SIZE_TYPE__ size_type;

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function declarations */
void use_all_types(void);
void opaque_use(void* ptr);

#endif /* TYPE_DEFS_H */
