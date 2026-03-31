#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* Scalar types (TYPE_SCALAR) */
typedef char scalar_char;
typedef short scalar_short;
typedef int scalar_int;
typedef long scalar_long;
typedef long long scalar_long_long;
typedef float scalar_float;
typedef double scalar_double;
typedef _Bool scalar_bool;
typedef _Complex float scalar_complex_float;
typedef _Complex double scalar_complex_double;
#ifdef __SIZEOF_INT128__
typedef __int128 scalar_int128;
typedef unsigned __int128 scalar_uint128;
#endif

/* String types (TYPE_STRING) */
typedef const char *string_const_ptr;
typedef char *string_mutable_ptr;

/* Basic struct (TYPE_STRUCT) */
struct basic_struct {
    int x;
    double y;
    char z;
} __attribute__((packed));

/* User struct (TYPE_USER_STRUCT) */
typedef struct basic_struct user_struct_t;

/* Complex nested struct */
struct nested_struct {
    struct basic_struct inner;
    struct nested_struct *next;
    int data[10];
};

/* Union types (TYPE_UNION) */
union basic_union {
    int as_int;
    float as_float;
    char as_char[4];
    void *as_ptr;
};

/* Tagged union */
struct tagged_union {
    enum { INT_TAG, FLOAT_TAG, STRING_TAG } tag;
    union {
        int int_val;
        float float_val;
        const char *string_val;
    } value;
};

/* Pointer types (TYPE_POINTER) */
typedef int *int_ptr_t;
typedef int **int_ptr_ptr_t;
typedef int ***int_ptr_ptr_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef int (*int_func_ptr_t)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef int int_matrix_3x3_t[3][3];
typedef int int_cube_2x2x2_t[2][2][2];
typedef const char *string_array_t[5];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_no_args_t)(void);
typedef void (*callback_with_args_t)(int, const char*, ...);
typedef void (*va_list_callback_t)(va_list);

/* Language-specific struct (TYPE_LANG_STRUCT) */
typedef __builtin_va_list va_list_struct_t;

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
        int id;
        char name[4];
    } identifier;
};

/* Function pointer array */
typedef void (*func_ptr_array_t[5])(void);

/* Self-referential types */
struct tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Opaque handle */
typedef struct undefined_struct *opaque_handle_t;

/* Complex type with all attributes */
struct __attribute__((aligned(16), packed)) aligned_packed_struct {
    char a;
    int b __attribute__((aligned(8)));
    double c;
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

/* Function declarations */
void use_all_types(void);
extern void external_function(void *ptr);

#endif /* TYPE_DEFS_H */
