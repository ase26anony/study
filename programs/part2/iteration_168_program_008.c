#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined;
struct another_undefined;
typedef struct undefined *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long long scalar_ll;
typedef unsigned int scalar_uint;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    scalar_int id;
    scalar_float value;
    char name[16];
};

struct complex_struct {
    struct simple_struct base;
    scalar_double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    scalar_float z;
} user_struct_t;

typedef struct nested_user {
    user_struct_t point;
    struct nested_user *child;
    struct nested_user *sibling;
} nested_user_t;

/* TYPE_UNION: Union types */
union basic_union {
    scalar_int as_int;
    scalar_float as_float;
    scalar_double as_double;
    void *as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_ll as_long;
    char as_string[64];
} __attribute__((packed));

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *scalar_ptr;
typedef struct simple_struct *struct_ptr;
typedef user_struct_t *user_struct_ptr;
typedef union basic_union *union_ptr;
typedef void *generic_ptr;
typedef const void *const_generic_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef scalar_int scalar_array[100];
typedef struct simple_struct struct_array[20];
typedef user_struct_t user_struct_array[50];
typedef union basic_union union_array[30];
typedef scalar_ptr pointer_array[40];
typedef string_ptr string_ptr_array[25];

/* Multi-dimensional arrays */
typedef int matrix_2d[10][10];
typedef float tensor_3d[5][5][5];
typedef struct simple_struct struct_matrix[3][3];

/* TYPE_CALLBACK: Function pointer types */
typedef scalar_int (*simple_callback)(void);
typedef void (*void_callback)(scalar_int, scalar_float);
typedef user_struct_t *(*struct_return_callback)(scalar_int param);
typedef scalar_int (*complex_callback)(struct simple_struct *, user_struct_t *, union basic_union);

/* Nested callback in struct */
struct callback_container {
    simple_callback cb1;
    void_callback cb2;
    complex_callback cb3;
    scalar_int (*inline_cb)(scalar_double);
};

/* TYPE_LANG_STRUCT: GCC internal structure patterns */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* GCC-specific attributes */
struct __attribute__((aligned(16))) aligned_struct {
    scalar_int data[4];
    scalar_double extra;
};

struct __attribute__((packed)) packed_struct {
    char a;
    scalar_int b;
    scalar_double c;
};

union __attribute__((transparent_union)) transparent_union_t {
    scalar_int *int_ptr;
    scalar_float *float_ptr;
    void *generic_ptr;
};

/* Complex nested type hierarchy */
typedef struct node {
    scalar_int value;
    struct node *left;
    struct node *right;
    union {
        scalar_int as_int;
        scalar_float as_float;
    } data;
    scalar_int (*compare)(struct node *, struct node *);
    struct node *children[];
} node_t;

/* Array of function pointers */
typedef scalar_int (*op_func)(scalar_int, scalar_int);
op_func operations[10];

/* Struct containing array of pointers to unions */
struct union_container {
    scalar_int count;
    union basic_union *items[50];
    void (*processor)(union basic_union **, scalar_int);
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*factory_func)(scalar_int id);

/* Deeply nested type example */
typedef struct outer {
    struct middle {
        struct inner {
            scalar_int depth;
            struct inner *next;
            void (*action)(scalar_int);
        } *inners;
        struct middle *chain;
        union {
            scalar_int tag;
            void *data;
        } info;
    } mids[5];
    scalar_int (*validator)(struct middle *);
    struct outer *parent;
} outer_t;

#endif /* TEST_TYPES_H */
