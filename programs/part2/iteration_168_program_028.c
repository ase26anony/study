#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef short scalar_short;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    scalar_int counter;
    scalar_float data[10];
    struct simple_struct nested;
    void* generic_ptr;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    double z;
} user_point_t;

typedef struct tagged_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
} tagged_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
    void* as_pointer;
};

union complex_union {
    struct {
        int type;
        char name[16];
    } header;
    struct {
        double x, y, z;
    } coordinates;
    struct {
        void* data;
        size_t size;
    } buffer;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef union simple_union* union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const char* const* string_ptr_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef void* pointer_array[50];
typedef char string_array[10][64];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*callback_int_int)(int);
typedef void (*callback_void_struct)(struct simple_struct*);
typedef double (*callback_double_args)(int, float, double);
typedef char* (*callback_string_union)(union simple_union*);
typedef void (*callback_complex)(int, struct complex_struct*, callback_int_int);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are typically defined in gcc/tree.h or similar headers */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_list;

/* Complex nested type definitions with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    int data[4];
    char padding[12];
};

union __attribute__((packed)) packed_union {
    int a;
    struct {
        char b;
        short c;
    } nested;
};

struct __attribute__((transparent_union)) transparent_union_wrapper {
    union {
        int* int_ptr;
        float* float_ptr;
        void* generic_ptr;
    } u;
};

/* Deeply nested type hierarchy */
typedef struct node {
    int value;
    struct node* left;
    struct node* right;
    void (*visit)(struct node*);
} tree_node_t;

typedef struct container {
    tree_node_t* root;
    int size;
    void (*add)(struct container*, int);
    int (*find)(struct container*, int);
    union {
        int as_array[100];
        tree_node_t* as_tree;
    } storage;
} container_t;

/* Function pointer returning pointer to struct containing callback */
typedef container_t* (*factory_func)(int size, callback_int_int validator);

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union simple_union* items[50];
    void (*processor)(union simple_union*, int);
};

/* Typedef for complex nested type */
typedef struct {
    int id;
    struct {
        callback_int_int validator;
        callback_void_struct processor;
    } callbacks;
    union_container* data;
    factory_func create_new;
} complex_system_t;

#endif /* TEST_TYPES_H */
