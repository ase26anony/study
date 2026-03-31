#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Never defined */
union forward_declared_union;    /* Never defined */

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
typedef __int128 int128_t;  /* If supported */

/* String type (TYPE_STRING) */
typedef char* string_ptr;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int x;
    double y;
    char z;
};

struct nested_struct {
    struct simple_struct inner;
    struct nested_struct* next;  /* Self-referential pointer */
};

struct bitfield_struct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 24;
    int d : 1;
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

struct aligned_struct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((aligned(32)));

/* Anonymous struct/union */
struct container {
    int tag;
    union {
        struct {
            int x;
            float y;
        } s;
        struct {
            double a;
            double b;
        } d;
    } data;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char name[50];
    void* data;
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int i;
    float f;
    double d;
    char* s;
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
    struct {
        int width, height;
    } rect;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_double_ptr;
typedef int*** int_triple_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*void_func_ptr)(void);
typedef int (*int_func_ptr)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef double matrix_3x3[3][3];
typedef char* string_array[5];
typedef void (*func_ptr_array[4])(void);

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback)(void* data, int result);
typedef char* (*string_formatter)(const char* fmt, ...);
typedef int (*va_func)(int count, ...);

/* Language struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list va_list_type;

/* Vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex type relationships */
struct graph_node {
    int id;
    struct graph_node** neighbors;  /* Array of pointers */
    int neighbor_count;
};

struct type_web {
    union simple_union* u_ptr;
    struct nested_struct nested;
    int_array_10 numbers;
    binary_op operation;
    struct type_web* next;
};

/* Function declarations */
void use_all_types(void);
extern void opaque_function(void* ptr);

#endif /* TYPE_DEFS_H */
