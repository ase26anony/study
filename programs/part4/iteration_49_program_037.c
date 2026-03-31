#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

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
typedef __int128 int128_t;  /* GNU extension */

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
    int outer_data;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
};

struct with_array_member {
    int ids[10];
    char name[50];
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
    } data;
};

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int user_id;
    char username[32];
} user_struct_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    struct {
        int int_value;
    } int_data;
    struct {
        float float_value;
    } float_data;
    struct {
        char* string_value;
    } string_data;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_matrix_3x3[3][3];
typedef int int_cube_2x2x2[2][2][2];
typedef char* string_array[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_no_args)(void);
typedef char* (*string_transform)(const char*, int);
typedef int (*variadic_func)(int, ...);

/* Language struct (TYPE_LANG_STRUCT) - using va_list */
typedef struct {
    va_list args;
    int count;
} lang_struct_t;

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex type relationships */
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

/* Function pointer arrays */
typedef int (*math_ops[4])(int, int);

/* Opaque type for cross-file testing */
struct opaque_type;

/* External declarations */
extern struct forward_declared_struct* global_forward_ptr;
extern union forward_declared_union* global_union_ptr;

#endif /* TYPE_DEFS_H */
