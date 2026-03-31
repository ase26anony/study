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
    scalar_int id;
    scalar_float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    scalar_double *dbl_ptr;
    int array[10][20];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    scalar_int x;
    scalar_float y;
    char label[64];
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t data;
    struct nested_user_struct *parent;
    void *user_data;
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    scalar_int as_int;
    scalar_float as_float;
    char as_char[4];
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_double as_double;
    void *as_pointer;
};

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef user_struct_t *user_ptr_t;
typedef union simple_union *union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const volatile char *cv_ptr_t;

/* TYPE_ARRAY: Arrays of different dimensions and types */
typedef scalar_int int_array_1d[100];
typedef scalar_float float_array_2d[10][20];
typedef struct simple_struct struct_array[50];
typedef user_struct_t *pointer_array[30];
typedef union complex_union union_array[5][5][5];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef scalar_int (*int_callback_t)(scalar_int, scalar_float);
typedef void (*void_callback_t)(struct simple_struct*, user_struct_t*);
typedef scalar_double (*complex_callback_t)(int_array_1d, void_callback_t);
typedef user_struct_t* (*factory_callback_t)(const char*, scalar_int);
typedef union simple_union (*union_callback_t)(scalar_double, scalar_int*);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Using patterns from GCC's internal tree representation */
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy struct with naming pattern that might be recognized */
struct GTY(()) lang_struct_dummy {
    int lang_specific_data;
    void *lang_hook;
};

/* Complex nested type hierarchy to ensure deep traversal */
typedef struct container_struct {
    /* Nested struct containing array of pointers to unions */
    union complex_union *union_ptr_array[20];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container* (*get_callback_container)(void);
    
    /* Multi-dimensional array of struct pointers */
    struct simple_struct *struct_ptr_matrix[5][5];
    
    /* Array of callbacks */
    int_callback_t callbacks[10];
    
    /* Self-referential pointer */
    struct container_struct *next;
} container_t;

struct callback_container {
    /* Struct containing callback members */
    void_callback_t on_start;
    void_callback_t on_end;
    factory_callback_t create_object;
    
    /* Nested union with function pointer */
    union {
        int_callback_t int_handler;
        complex_callback_t complex_handler;
    } handler;
    
    /* Pointer to array of containers */
    container_t **containers;
};

/* Apply GCC attributes to influence gengtype categorization */
struct __attribute__((aligned(16), packed)) attributed_struct {
    scalar_int a;
    scalar_double b;
    char c;
} __attribute__((aligned(32)));

union __attribute__((transparent_union)) transparent_union_t {
    scalar_int *int_ptr;
    scalar_float *float_ptr;
    void *generic_ptr;
};

typedef struct __attribute__((packed)) packed_user_struct {
    scalar_int x;
    scalar_double y;
    char z;
} packed_user_t;

/* Even more complex nesting for thorough traversal */
typedef struct ultimate_nest {
    /* Pointer to function returning pointer to array of callbacks */
    struct callback_container* (**(*complex_func_ptr)(int))[10];
    
    /* Array of pointers to unions containing structs with callbacks */
    union {
        struct callback_container cb_container;
        container_t container;
        user_struct_t user;
    } *variant_array[15];
    
    /* Multi-level pointer */
    scalar_int ****quad_ptr;
    
    /* Callback that takes callback as parameter */
    scalar_int (*meta_callback)(int_callback_t, void_callback_t);
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int flag3 : 4;
        scalar_int : 24; /* padding */
    } flags;
    
    /* Flexible array member */
    scalar_double data[];
} ultimate_nest_t;

/* Additional undefined types for TYPE_UNDEFINED coverage */
extern struct forward_declared *global_forward_ptr;
typedef void incomplete_array[];

#endif /* TEST_TYPES_H */
