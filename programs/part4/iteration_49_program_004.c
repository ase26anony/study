#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Forward declarations (TYPE_UNDEFINED) */
struct forward_declared_struct;
union forward_declared_union;

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
typedef __int128 int128_type __attribute__((aligned(16)));

/* String types (TYPE_STRING) */
typedef char* string_type;
typedef const char* const_string_type;

/* Function pointer types (TYPE_CALLBACK) */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, ...);
typedef int (*math_callback)(double, double);
typedef void (*void_callback)(void);

/* Struct types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct nested_struct {
    struct simple_struct inner;
    int outer_value;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
} __attribute__((packed));

struct array_member_struct {
    int ids[10];
    char name[50];
    float matrix[3][3];
};

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
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

/* Anonymous struct/union */
struct container {
    union {
        int x;
        float y;
    } anonymous_union;
    struct {
        char a;
        char b;
    } anonymous_struct;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_ptr_ptr;
typedef int*** int_ptr_ptr_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*func_ptr)(void);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef char char_array_2d[5][10];
typedef float float_array_3d[3][3][3];
typedef struct simple_struct struct_array[5];
typedef union simple_union union_array[8];

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Language-specific struct (TYPE_LANG_STRUCT placeholder) */
/* This would typically be GCC internal types, but we can hint at it */
typedef __builtin_va_list va_list_type;

/* User struct (TYPE_USER_STRUCT) - marked with attribute */
struct __attribute__((aligned(64))) aligned_struct {
    double data[8];
    char padding[64];
};

/* Complex type dependencies */
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

/* Function declarations */
void use_all_types(void);
extern void external_use(void* ptr);

#endif /* TYPE_DEFS_H */
