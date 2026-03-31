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

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double matrix[4][4];
    void* metadata;
    struct complex_struct* next;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
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
typedef void (*void_func_ptr)(void);
typedef const volatile char* special_ptr;

/* TYPE_ARRAY: Arrays of different dimensions and types */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[8];
typedef void* pointer_array[16];
typedef int (*func_ptr_array[4])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback)(void);
typedef void (*void_callback)(int, float, char*);
typedef struct simple_struct* (*struct_callback)(int param1, double param2);
typedef union simple_union (*union_callback)(const char* str, ...);
typedef int (*complex_callback)(int (*nested)(float), void* context);

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node) */
struct tree_node;
typedef struct tree_node* tree_ptr;

/* Complex nested type definitions */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    struct {
        union nested_union* items[10];
        int count;
    } union_container;
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_holder* (*get_callback_holder)(int id);
    
    /* Array of callbacks */
    int_callback callbacks[5];
    
    /* Pointer to array of structs */
    tagged_t (*tagged_array)[];
} container_t;

struct callback_holder {
    void_callback handler;
    void* user_data;
    struct callback_holder* next;
};

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    long long timestamp;
};

union __attribute__((transparent_union)) transparent_union_t {
    int* as_int_ptr;
    float* as_float_ptr;
    void* as_void_ptr;
};

/* More complex nesting examples */
typedef struct node {
    int value;
    struct node* left;
    struct node* right;
    void (*visit)(struct node*);
} tree_node_t;

typedef union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
            void* ptr_val;
            int (*func_val)(int);
        } data;
    } tagged;
    unsigned char raw_data[32];
} variant_t;

/* Multi-level pointer types */
typedef int**** complex_pointer;
typedef struct container**** deep_container_ptr;

/* Array of function pointers with different signatures */
typedef void (*multi_callback[3])(
    int,
    struct simple_struct*,
    union simple_union
);

/* Self-referential structures */
struct self_ref {
    int data;
    struct self_ref* next;
    struct self_ref* (*clone)(struct self_ref*);
};

/* Const volatile qualified types */
typedef const volatile int cv_int;
typedef const struct simple_struct* const_struct_ptr;
typedef volatile union simple_union* volatile_union_ptr;

/* Anonymous struct/union in typedef */
typedef struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int radius;
        struct {
            int width;
            int height;
        } rect;
    } shape;
} geometry_t;

#endif /* TEST_TYPES_H */
