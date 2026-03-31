#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef unsigned int scalar_uint;

/* TYPE_STRING: String-related types */
typedef const char* string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int id;
    float value;
    char name[20];
};

struct complex_struct {
    struct simple_struct base;
    double extra_data;
    struct complex_struct *next;
    int flags[10];
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    char label[50];
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t data;
    struct nested_user_struct *parent;
    void *user_data;
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    union simple_union as_simple;
    double as_array[4];
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef union complex_union *union_ptr;
typedef user_struct_t *user_struct_ptr;
typedef void (*func_ptr)(void);
typedef const void *const_void_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_1d[10];
typedef int int_array_2d[5][5];
typedef struct simple_struct struct_array[8];
typedef user_struct_t *pointer_array[20];
typedef union simple_union union_array[15];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*callback_int_int)(int);
typedef void (*callback_void_struct)(struct simple_struct*);
typedef double (*callback_double_args)(int, float, const char*);
typedef user_struct_t* (*callback_user_creator)(int, const char*);
typedef void (*callback_complex)(struct complex_struct*, union complex_union*, callback_int_int);

/* TYPE_LANG_STRUCT: GCC internal structure (pattern recognized by gengtype) */
struct tree_node;
struct tree_common;
typedef struct tree_node *tree;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
    int counter;
};

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    long *long_ptr;
    void *void_ptr;
};

/* Complex nested type hierarchy */
typedef struct container {
    /* Mixed member types */
    int scalar_member;
    char string_member[64];
    
    /* Nested struct */
    struct {
        int nested_id;
        float nested_value;
    } inner;
    
    /* Array of pointers to unions */
    union complex_union *union_ptrs[5];
    
    /* Function pointer member */
    callback_double_args compute_func;
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Pointer to another container */
    struct container *next;
    
    /* Callback array */
    callback_int_int handlers[3];
} container_t;

/* Even more complex: struct containing array of structs containing callbacks */
typedef struct node {
    int id;
    struct node *children[4];
    void (*action)(struct node*, int);
    union {
        int int_val;
        double double_val;
        char *string_val;
    } data;
} node_t;

/* Function pointer returning pointer to struct containing callback */
typedef container_t* (*factory_func)(int, const char*, callback_int_int);

/* Typedef chain for deep traversal */
typedef int base_type;
typedef base_type derived_type;
typedef derived_type *derived_ptr;
typedef derived_ptr (*derived_callback)(derived_type);

#endif /* TEST_TYPES_H */
