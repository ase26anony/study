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

/* TYPE_STRUCT: Complete struct definitions */
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
    char label[64];
} user_struct_t;

typedef struct nested_user {
    user_struct_t data;
    struct nested_user *parent;
    struct nested_user *children[10];
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
    long long as_ll;
} __attribute__((packed));

/* GCC-specific transparent union */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr_t;
typedef struct simple_struct *struct_ptr_t;
typedef user_struct_t *user_ptr_t;
typedef union simple_union *union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const char *const *string_ptr_ptr;

/* TYPE_ARRAY: Arrays of different dimensions and types */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef user_struct_t *pointer_array[15];
typedef int (*func_ptr_array[8])(void);

/* Multi-dimensional complex array */
typedef union complex_union nested_array[3][3][3];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*callback_int_void)(void);
typedef void (*callback_void_int)(int);
typedef float (*callback_float_two_params)(int, float);
typedef struct simple_struct *(*callback_struct_ptr)(user_struct_t*, int);
typedef void (*callback_complex)(int, float, const char*, void*);

/* Complex nested callback type */
typedef callback_int_void (*callback_returning_callback)(int);

/* TYPE_LANG_STRUCT: GCC internal structure (tree node) */
struct tree_node;
struct tree_common;
typedef struct tree_node *tree_ptr;

/* GCC internal-like structure with attributes */
struct lang_type {
    struct tree_node *type;
    unsigned int lang_flag : 1;
    unsigned int has_asm_name : 1;
} __attribute__((aligned(16)));

struct lang_decl {
    struct tree_node *decl;
    const char *section_name;
} __attribute__((packed));

/* Complex nested type combining multiple categories */
typedef struct ultimate_nested_type {
    /* Scalar members */
    scalar_int id;
    scalar_double precision;
    
    /* String member */
    const char *type_name;
    
    /* Struct member */
    user_struct_t metadata;
    
    /* Union member */
    union complex_union variant;
    
    /* Pointer members */
    struct ultimate_nested_type *self_ptr;
    void *opaque_data;
    callback_int_void handler;
    
    /* Array members */
    int matrix[3][3];
    user_struct_t items[5];
    func_ptr_array callbacks;
    
    /* Nested struct with callback */
    struct {
        int priority;
        callback_complex action;
    } task;
    
    /* Flexible array member */
    int dynamic_data[];
} ultimate_nested_t;

/* Function pointer returning complex nested type */
typedef ultimate_nested_t *(*factory_callback)(int, const char*);

/* Array of factory callbacks */
factory_callback factories[4];

/* Global variables using the types for reference */
extern scalar_int global_counter;
extern string_ptr global_message;
extern struct simple_struct global_struct;
extern user_struct_t global_user_struct;
extern union simple_union global_union;
extern int_array_1d global_array;
extern callback_int_void global_callback;
extern struct lang_type *global_lang_struct;

#endif /* TEST_TYPES_H */
