#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef long scalar_long_t;
typedef unsigned int scalar_uint_t;

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr_t;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double matrix[4][4];
    void *generic_ptr;
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    char label[64];
} user_point_t;

typedef struct nested_user {
    user_point_t points[10];
    struct nested_user *parent;
    void (*callback)(int);
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            double dbl_val;
            char *str_val;
        } data;
    } tagged;
    unsigned char raw_data[16];
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef union simple_union* union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef int (*math_func_t)(int, int);

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10_t[10];
typedef struct simple_struct struct_array_5_t[5];
typedef void* ptr_array_20_t[20];
typedef int multi_dim_array_t[3][4][5];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*binary_op_t)(int, int);
typedef void (*void_callback_t)(void);
typedef char* (*string_processor_t)(const char*, int);
typedef struct simple_struct* (*struct_factory_t)(int, float);
typedef void (*complex_callback_t)(int, double, const char*, void*);

/* TYPE_LANG_STRUCT: GCC internal-like structures */
/* These mimic GCC's internal tree representation */
struct tree_common {
    int code;
    union {
        int int_cst;
        double real_cst;
        char *string_cst;
    } u;
};

struct tree_type {
    struct tree_common common;
    unsigned int precision : 16;
    unsigned int machine_mode : 8;
    unsigned int unsigned_flag : 1;
    unsigned int volatile_flag : 1;
};

struct tree_node {
    struct tree_common common;
    union {
        struct tree_type type;
        struct {
            struct tree_node *operands[3];
        } expr;
    } u;
};

/* Complex nested type combining multiple categories */
typedef struct container {
    /* TYPE_STRUCT */
    int id;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_UNION */
    union simple_union *union_ptrs[8];
    
    /* TYPE_CALLBACK member */
    binary_op_t operation;
    
    /* TYPE_STRING */
    const char *description;
    
    /* Nested TYPE_STRUCT */
    struct {
        user_point_t position;
        double timestamp;
    } metadata;
    
    /* TYPE_ARRAY of TYPE_USER_STRUCT */
    user_point_t trajectory[100];
    
    /* TYPE_POINTER to TYPE_CALLBACK */
    complex_callback_t *callback_ptr;
    
    /* TYPE_UNION */
    union {
        int mode;
        struct {
            unsigned int flags;
            void *context;
        } state;
    } config;
} container_t;

/* Another complex type with deep nesting */
typedef struct graph_node {
    int value;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRUCT */
    struct graph_node **neighbors;
    int neighbor_count;
    
    /* TYPE_CALLBACK for node processing */
    void (*visit)(struct graph_node*);
    
    /* TYPE_UNION for node data */
    union node_data {
        int int_data;
        double float_data;
        char *string_data;
        void *opaque_data;
    } data;
} graph_node_t;

/* Function pointer returning pointer to struct containing callbacks */
typedef container_t* (*container_factory_t)(
    int id,
    const char* desc,
    binary_op_t op
);

/* Array of function pointers */
typedef binary_op_t op_array_t[10];

/* Struct with transparent union attribute (GCC-specific) */
struct with_transparent_union {
    int type;
    union __attribute__((transparent_union)) {
        int *int_ptr;
        float *float_ptr;
        char *char_ptr;
    } data;
};

/* Packed struct with alignment (GCC-specific) */
struct __attribute__((packed, aligned(4))) packed_struct {
    char a;
    int b;
    short c;
    double d;
};

/* Forward declaration for mutual recursion */
struct mutually_recursive_a;
struct mutually_recursive_b;

struct mutually_recursive_a {
    int value;
    struct mutually_recursive_b *partner;
};

struct mutually_recursive_b {
    char tag;
    struct mutually_recursive_a *partner;
};

#endif /* TEST_TYPES_H */
