#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

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
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4;  /* unnamed bitfield */
    unsigned int value : 8;
};

struct array_member_struct {
    int numbers[10];
    char name[32];
    float matrix[3][3];
};

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

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char name[50];
    void* metadata;
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char c;
    void* p;
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
    };
} __attribute__((aligned(16)));

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][5];
typedef float float_array_3d[3][3][3];
typedef void_func_ptr func_ptr_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_no_args)(void);
typedef char* (*string_transform)(const char*, int);
typedef void (*variadic_func)(int, ...);
typedef int (*complex_callback)(struct nested_struct*, union tagged_union**, va_list);

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language struct placeholder (TYPE_LANG_STRUCT) */
/* This would typically be GCC internal types */

/* Function declarations */
void use_all_types(void);
extern void external_func(void*);

#endif /* TYPE_DEFS_H */
