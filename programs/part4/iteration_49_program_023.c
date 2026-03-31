#ifndef TYPE_DEFS_H
#define TYPE_DEFS_H

#include <stdarg.h>

/* Undefined/Incomplete types (TYPE_UNDEFINED) */
struct forward_declared_struct;  /* Will never be defined */
union forward_declared_union;     /* Will never be defined */

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
typedef __int128 my_int128_t;
typedef unsigned __int128 my_uint128_t;

/* String types (TYPE_STRING) */
typedef char* string_ptr_t;
typedef const char* const_string_ptr_t;

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
} __attribute__((packed));

/* User struct types (TYPE_USER_STRUCT) */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct {
    point_t start;
    point_t end;
    double thickness;
} line_t;

/* Union types (TYPE_UNION) */
union simple_union {
    int as_int;
    float as_float;
    char as_chars[4];
};

union tagged_union {
    int type;
    struct {
        int type;
        int value;
    } integer;
    struct {
        int type;
        double value;
    } floating;
};

/* Pointer types (TYPE_POINTER) */
typedef int* int_ptr_t;
typedef int** int_ptr_ptr_t;
typedef int*** int_ptr_ptr_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef union simple_union* union_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef int (*int_func_ptr_t)(int, int);

/* Array types (TYPE_ARRAY) */
typedef int int_array_10_t[10];
typedef char char_array_2d_t[5][10];
typedef struct simple_struct struct_array_t[3];
typedef union simple_union union_array_t[4];

/* Callback types (TYPE_CALLBACK) */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(int, void*);
typedef void (*varargs_callback_t)(int, ...);
typedef int (*complex_callback_t)(int (*)(int), void*);

/* Language struct types (TYPE_LANG_STRUCT) */
typedef __builtin_va_list va_list_t;

/* Vector types (GNU extension) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Complex type relationships */
struct node {
    int data;
    struct node* next;
    struct node* prev;
};

struct tree_node {
    int value;
    struct tree_node* left;
    struct tree_node* right;
};

struct graph_node {
    int id;
    struct graph_node** neighbors;
    int neighbor_count;
};

/* Function pointer arrays */
typedef int (*op_func_t)(int, int);
extern op_func_t operations[];

/* Anonymous structs/unions */
struct container {
    int type;
    union {
        int i;
        float f;
        char str[16];
    } data;
    struct {
        int x;
        int y;
    } position;
};

/* Extern declarations for cross-file usage */
extern struct simple_struct global_struct;
extern union simple_union global_union;
extern int_array_10_t global_array;

/* Function declarations using various types */
void process_int_ptr(int_ptr_t ptr);
void process_struct_ptr(struct_ptr_t ptr);
void process_union_ptr(union_ptr_t ptr);
void process_callback(binary_op_t op);
int execute_operation(binary_op_t op, int a, int b);
void handle_varargs(varargs_callback_t cb, ...);

#endif /* TYPE_DEFS_H */
