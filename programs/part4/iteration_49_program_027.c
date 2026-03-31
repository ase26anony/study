#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

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
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

struct array_member_struct {
    int numbers[10];
    char name[32];
    struct nested_struct nested;
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
typedef struct simple_struct user_struct_t;
typedef struct nested_struct nested_user_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    char c;
    double d;
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
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef void (*void_func_ptr)(void);
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_array_2d[5][5];
typedef int int_array_3d[3][3][3];
typedef struct simple_struct struct_array[5];
typedef void_func_ptr func_ptr_array[10];

/* Callback types (TYPE_CALLBACK) */
typedef int (*int_callback)(int);
typedef void (*void_callback)(void);
typedef char* (*string_callback)(const char*, int);
typedef int (*variadic_callback)(int, ...);
typedef void (*complex_callback)(struct nested_struct*, union tagged_union*);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list va_list_type;
typedef size_t size_type;
typedef ptrdiff_t ptrdiff_type;

/* Vector types */
typedef int v4si __attribute__ ((vector_size (16)));
typedef float v4sf __attribute__ ((vector_size (16)));

/* Function pointer with attributes */
typedef void (*noreturn_func)(void) __attribute__((noreturn));
typedef int (*pure_func)(int) __attribute__((pure));

/* Opaque incomplete type */
extern struct opaque_type;
typedef struct opaque_type* opaque_ptr;

/* Self-referential structures */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    struct tree_node* parent;
};

struct linked_list {
    void* data;
    struct linked_list* next;
    struct linked_list* prev;
};

#endif /* TYPE_DEFS_H */
