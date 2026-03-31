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
    user_struct_t data;
    struct nested_user_struct *parent;
    void *user_data;
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    scalar_int as_int;
    scalar_float as_float;
    scalar_double as_double;
    char as_char;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_double as_array[8];
    void *as_pointer;
};

/* GCC-specific union attribute */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef user_struct_t *user_struct_ptr_t;
typedef union simple_union *union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const char *const *string_ptr_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef scalar_int int_array_1d[10];
typedef scalar_float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef user_struct_t *pointer_array[50];
typedef union complex_union union_array[15];

/* Multi-dimensional complex array */
typedef struct {
    int matrix[3][3];
    user_struct_t items[4];
    void *pointers[2];
} array_container_t;

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef scalar_int (*int_callback_t)(scalar_int, scalar_float);
typedef void (*void_callback_t)(struct simple_struct*, user_struct_t*);
typedef scalar_double (*complex_callback_t)(int_array_1d, union_ptr_t, string_ptr);
typedef user_struct_t* (*struct_return_callback_t)(void);
typedef void (*varargs_callback_t)(const char*, ...);

/* Struct containing callback members */
struct callback_container {
    int_callback_t int_handler;
    void_callback_t void_handler;
    struct_return_callback_t struct_factory;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structure */
/* Using pattern similar to GCC's internal tree nodes */
struct tree_common {
    int code;
    struct tree_common *chain;
    struct tree_common *type;
};

struct tree_node {
    struct tree_common common;
    union {
        struct undefined_struct *ptr;
        scalar_int val;
        string_ptr str;
    } u;
};

/* Complex nested type hierarchy */
typedef struct {
    /* Nested struct containing array of pointers to unions */
    struct {
        union complex_union *union_ptrs[8];
        int count;
    } union_container;
    
    /* Function pointer returning pointer to struct containing callback */
    struct callback_container* (*get_callbacks)(void);
    
    /* Multi-dimensional array of structs */
    user_struct_t user_grid[4][4];
    
    /* Pointer to array of function pointers */
    int_callback_t (*callback_array)[5];
    
    /* Self-referential pointer */
    struct nested_type *self;
} nested_type;

/* GCC attributes on structs */
struct __attribute__((aligned(16), packed)) aligned_packed_struct {
    char a;
    int b;
    double c;
};

/* Final complex type that references everything */
typedef struct master_type {
    /* All type categories referenced */
    undefined_ptr_t undefined_ref;
    scalar_int scalar_field;
    string_ptr string_field;
    struct simple_struct struct_field;
    user_struct_t user_field;
    union simple_union union_field;
    nested_type *pointer_field;
    int_array_1d array_field;
    struct callback_container callbacks;
    struct tree_node *lang_struct_field;
    
    /* Nested arrays and pointers */
    struct complex_struct *struct_ptr_array[10];
    union complex_union (*union_ptr_matrix)[3][3];
    
    /* Complex callback signature */
    nested_type* (*complex_factory)(
        int param1, 
        struct master_type *param2,
        int (*comparator)(const void*, const void*)
    );
} master_type_t;

#endif /* TEST_TYPES_H */
