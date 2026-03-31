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

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
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
    void* data_ptr;
    int flags;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    int z;
} user_vec3_t;

typedef struct tagged_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
} tagged_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union nested_union {
    struct {
        int type;
        union simple_union data;
    } tagged;
    long long as_llong;
    double as_double[2];
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*func_ptr)(void);
typedef const volatile char* special_ptr;

/* TYPE_ARRAY: Arrays of different dimensions and types */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[8];
typedef void* ptr_array[16];
typedef char string_array[4][64];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*callback_int_int)(int);
typedef void (*callback_void_struct)(struct simple_struct*);
typedef float (*callback_float_args)(int, float, double);
typedef struct complex_struct* (*callback_struct_return)(void);
typedef void (*callback_complex)(int, struct simple_struct*, callback_int_int);

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node is from GCC's internal representation) */
struct tree_node;
struct tree_common;
typedef struct tree_node* tree_ptr;

/* Complex nested type definitions with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
    double d;
};

union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    long* long_ptr;
    void* void_ptr;
};

/* Deeply nested type hierarchy */
typedef struct node {
    int value;
    struct node* next;
    struct node* prev;
    void (*print)(struct node*);
} node_t;

typedef struct container {
    node_t* nodes[10];
    union nested_union storage;
    callback_int_int processor;
    string_ptr names[5];
} container_t;

/* Function pointer returning pointer to struct containing callback */
typedef container_t* (*factory_func)(int, const char*);

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union simple_union* unions[8];
    void (*process)(union simple_union*);
};

/* Typedef for complex nested type */
typedef struct {
    struct union_container uc;
    factory_func create;
    int_array_1d indices;
    tagged_t metadata;
} super_container_t;

#endif /* TEST_TYPES_H */
