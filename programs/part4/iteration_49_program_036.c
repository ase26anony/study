#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

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
typedef const char *string_type;

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
    unsigned int padding : 24;
} __attribute__((packed));

struct array_member_struct {
    int ids[10];
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
    } value;
};

/* User struct (TYPE_USER_STRUCT) */
typedef struct simple_struct simple_struct_t;
typedef struct nested_struct nested_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char *s;
};

union tagged_union {
    enum { INT, FLOAT, STRING } tag;
    struct {
        int type;
        union {
            int i;
            float f;
            char *s;
        } value;
    } data;
};

/* Pointer types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef int **int_ptr_ptr_t;
typedef int ***int_ptr_ptr_ptr_t;
typedef const int *const_int_ptr_t;
typedef volatile char *volatile_char_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef char char_array_2d_t[5][10];
typedef float float_3d_array_t[3][3][3];
typedef struct simple_struct struct_array_t[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*int_callback_t)(int, int);
typedef void (*void_callback_t)(void);
typedef char *(*string_callback_t)(const char *, ...);
typedef void (*va_callback_t)(int, va_list);
typedef int (*complex_callback_t)(struct simple_struct *, union simple_union *);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list va_list_type;
typedef __SIZE_TYPE__ size_type;

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Function declarations */
void use_all_types(void);
void opaque_use(void *ptr);

#endif /* TYPE_DEFS_H */
