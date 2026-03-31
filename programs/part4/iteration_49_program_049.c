#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>
#include <stddef.h>

/* Undefined/Incomplete Types (TYPE_UNDEFINED) */
struct undefined_struct;  /* Forward declaration */
union undefined_union;    /* Forward declaration */

/* Scalar Types (TYPE_SCALAR) */
typedef char my_char;
typedef short my_short;
typedef int my_int;
typedef long my_long;
typedef long long my_longlong;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;
typedef _Complex float my_complex_float;
typedef _Complex double my_complex_double;
typedef __int128 my_int128;  /* GNU extension */

/* String Types (TYPE_STRING) */
typedef char* my_string;
typedef const char* my_const_string;

/* Struct Types (TYPE_STRUCT) */
struct simple_struct {
    int a;
    char b;
    double c;
} __attribute__((packed));

struct nested_struct {
    struct simple_struct inner;
    int outer_value;
};

struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

struct array_member_struct {
    int id;
    char name[32];
    float values[16];
};

/* User Struct (TYPE_USER_STRUCT) - via typedef */
typedef struct {
    int x;
    int y;
} point_t;

/* Union Types (TYPE_UNION) */
union data_union {
    int i;
    float f;
    char str[4];
};

union tagged_union {
    int type;
    struct {
        int type;
        int value;
    } integer;
    struct {
        int type;
        float value;
    } floating;
};

/* Pointer Types (TYPE_POINTER) */
typedef int* int_ptr;
typedef int** int_double_ptr;
typedef int*** int_triple_ptr;
typedef struct simple_struct* struct_ptr;
typedef void (*void_func_ptr)(void);

/* Array Types (TYPE_ARRAY) */
typedef int int_array_10[10];
typedef int int_matrix_3x3[3][3];
typedef int int_cube_2x2x2[2][2][2];
typedef char* string_array[5];

/* Callback Types (TYPE_CALLBACK) */
typedef int (*binary_op)(int, int);
typedef void (*callback_func)(void* data, int result);
typedef int (*va_func)(int count, ...);

/* Language Struct (TYPE_LANG_STRUCT) - using builtin types */
typedef __builtin_va_list my_va_list;
typedef size_t my_size_t;
typedef ptrdiff_t my_ptrdiff_t;

/* Vector Types (GNU extension) */
typedef int v4si __attribute__ ((vector_size (16)));

/* Complex type relationships */
struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
    void (*print)(struct tree_node*);
};

struct linked_list {
    int data;
    struct linked_list* next;
    struct linked_list* prev;
};

/* Function pointer arrays */
typedef int (*math_ops[4])(int, int);

/* Anonymous struct/union */
struct container {
    union {
        int as_int;
        float as_float;
    };
    struct {
        char tag;
        char data[15];
    };
};

/* Opaque external references */
extern struct undefined_struct* external_ref;

#endif /* TYPE_DEFS_H */
