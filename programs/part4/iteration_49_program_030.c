#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Will never be defined */
union forward_declared_union;    /* Will never be defined */

/* Scalar types (TYPE_SCALAR) */
typedef int my_int_t;
typedef char my_char_t;
typedef short my_short_t;
typedef long my_long_t;
typedef float my_float_t;
typedef double my_double_t;
typedef _Bool my_bool_t;
typedef _Complex float my_complex_t;
typedef _Complex double my_dcomplex_t;

/* String type (TYPE_STRING) */
typedef char* string_t;

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    float c;
    double d;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    long extra;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int : 24;  /* padding */
};

struct array_member_struct {
    int numbers[10];
    char name[32];
    float matrix[3][3];
};

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
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef void (*generic_func_ptr_t)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef float matrix_3x3_t[3][3];
typedef char* string_array_t[5];
typedef int (*func_ptr_array_t[10])(int, int);

/* Callback/Function pointer types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef char* (*string_formatter_t)(const char* fmt, ...);
typedef void (*va_callback_t)(const char* fmt, va_list args);

/* User struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int id;
    char name[50];
    void* data;
} user_struct_t;

/* Language struct (TYPE_LANG_STRUCT) - using GCC extensions */
struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
};

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* __int128 if available */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#endif

/* Builtin types */
typedef __builtin_va_list va_list_t;

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

/* Anonymous struct/union */
struct container {
    int type;
    union {
        int int_val;
        float float_val;
        struct {
            char* str;
            int len;
        } string_val;
    } data;
};

/* Function declarations */
void use_all_types(void);

#endif /* TYPE_DEFS_H */
