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
typedef unsigned int scalar_uint;

/* TYPE_STRING: String-related types */
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
    scalar_double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    scalar_int x;
    scalar_float y;
    char label[64];
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t user;
    struct nested_user_struct *parent;
    int depth;
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
typedef scalar_int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef user_struct_t *user_ptr;
typedef union simple_union *union_ptr;
typedef void (*func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef scalar_int int_array[100];
typedef struct simple_struct struct_array[50];
typedef user_struct_t *pointer_array[20];
typedef int multi_dim_array[3][4][5];
typedef const char *string_array[];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef scalar_int (*int_callback)(scalar_int, scalar_float);
typedef void (*void_callback)(struct simple_struct*, user_struct_t*);
typedef user_struct_t* (*struct_callback)(int, char**);
typedef union complex_union (*union_callback)(scalar_double);
typedef int (*nested_callback)(int (*)(int), void*);

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node is from GCC's internal representation) */
struct tree_node;
typedef struct tree_node *tree_ptr;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    scalar_char a;
    scalar_int b;
    scalar_char c;
};

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    long *long_ptr;
};

struct __attribute__((aligned(64))) aligned_struct {
    scalar_double data[8];
    scalar_int metadata;
};

/* Complex nested type hierarchy */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    union complex_union *union_array[10];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container* (*get_callback_container)(void);
    
    /* Multi-dimensional array of struct pointers */
    struct simple_struct *struct_grid[5][5];
    
    /* Callback member */
    int_callback processor;
    
    /* Self-referential pointer */
    struct container *next;
} container_t;

struct callback_container {
    /* Struct containing callback members */
    void_callback on_start;
    struct_callback on_data;
    nested_callback on_complete;
    
    /* Array of function pointers */
    int_callback handlers[5];
    
    /* Pointer to container */
    container_t *owner;
};

/* Even more complex nesting */
typedef struct ultimate_nest {
    container_t main_container;
    struct callback_container *callbacks[3];
    union {
        aligned_struct aligned;
        packed_struct packed;
    } variant;
    
    /* Pointer to function returning pointer to array of pointers */
    container_t* (*(*complex_func_ptr)(void))[10];
    
    /* Nested anonymous struct */
    struct {
        tree_ptr gcc_internal;
        undefined_ptr_t undefined;
        scalar_int counters[100];
    } internal;
} ultimate_nest_t;

/* Global variables using these types (for reference in test program) */
extern const scalar_int GLOBAL_SCALAR;
extern string_ptr GLOBAL_STRING;
extern struct simple_struct GLOBAL_STRUCT;
extern user_struct_t GLOBAL_USER_STRUCT;
extern union simple_union GLOBAL_UNION;
extern int_array GLOBAL_ARRAY;
extern int_callback GLOBAL_CALLBACK;

#endif /* TEST_TYPES_H */
