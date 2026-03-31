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
typedef long long scalar_ll;
typedef unsigned int scalar_uint;

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    scalar_int id;
    scalar_float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    scalar_double *dbl_ptr;
    int matrix[4][4];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    scalar_int x;
    scalar_float y;
    char tag;
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
    void *as_pointer;
    scalar_double as_array[8];
};

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef user_struct_t *user_ptr_t;
typedef union simple_union *union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const volatile void *cv_void_ptr_t;

/* TYPE_ARRAY: Arrays of different dimensions and types */
typedef scalar_int int_array_1d[10];
typedef scalar_float float_array_2d[5][5];
typedef struct simple_struct struct_array[8];
typedef user_struct_t *pointer_array[16];
typedef int (*func_ptr_array[4])(void);

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback_t)(scalar_int, scalar_float);
typedef void (*void_callback_t)(void);
typedef struct simple_struct *(*struct_callback_t)(user_struct_t*, int);
typedef scalar_double (*complex_callback_t)(int, float, const char*, void*);
typedef void (*nested_callback_t)(int (*)(float), void (*)(void));

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Using patterns that might be recognized by gengtype */
struct tree_common;
struct tree_decl_common;
struct tree_type_common;

/* GCC-specific tree node pattern */
struct tree_node {
    struct tree_node *chain;
    struct tree_node *next;
    int code;
    union {
        long intval;
        double realval;
        char *strval;
        void *ptrval;
    } u;
} __attribute__((aligned(16)));

/* Complex nested type definitions with GCC attributes */
typedef struct __attribute__((packed)) packed_struct {
    scalar_int a;
    scalar_char b;
    scalar_float c;
} packed_struct_t;

typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
} transparent_union_t;

/* Deeply nested type hierarchy */
typedef struct container {
    /* Mix of all type categories */
    scalar_int count;                     /* TYPE_SCALAR */
    string_ptr description;               /* TYPE_STRING */
    struct simple_struct data;            /* TYPE_STRUCT */
    user_struct_t user_data;              /* TYPE_USER_STRUCT */
    union complex_union variant;          /* TYPE_UNION */
    int_array_1d numbers;                 /* TYPE_ARRAY */
    int_callback_t callback;              /* TYPE_CALLBACK */
    struct tree_node *tree_node;          /* TYPE_LANG_STRUCT */
    
    /* Nested pointers and arrays */
    void *pointers[4];                    /* TYPE_POINTER in TYPE_ARRAY */
    struct container *next;               /* TYPE_POINTER to TYPE_STRUCT */
    
    /* Function pointer returning pointer to struct with callback */
    struct container *(*finder)(int, int_callback_t);
    
    /* Array of pointers to unions */
    union simple_union *union_ptrs[8];
    
    /* Callback returning pointer to struct containing callback */
    struct container *(*complex_finder)(
        int, 
        struct container *(*)(int, int_callback_t)
    );
} container_t;

/* Additional complex typedef */
typedef container_t *(*container_factory_t)(
    const char*, 
    int, 
    void (*)(container_t*)
);

/* Struct with array of function pointers */
struct callback_registry {
    const char *name;
    int_callback_t callbacks[10];
    void_callback_t void_callbacks[5];
    struct callback_registry *registries[3];
};

/* Union containing struct with array of pointers */
union mega_union {
    struct {
        int type;
        void *data[10];
        union mega_union *next;
    } linked;
    container_t container;
    struct callback_registry registry;
};

/* Forward declaration for circular reference */
struct circular;
struct circular {
    int value;
    struct circular *next;
    struct circular *prev;
};

#endif /* TEST_TYPES_H */
