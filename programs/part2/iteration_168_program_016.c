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
    scalar_int counter;
    scalar_float data[10];
    struct simple_struct nested;
    char description[256];
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    double z;
} user_point_t;

typedef struct user_named_struct {
    user_point_t position;
    string_ptr name;
    int flags;
} user_named_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

union complex_union {
    struct simple_struct as_struct;
    user_point_t as_point;
    void* as_pointer;
    long as_long;
} __attribute__((aligned(16)));

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*void_func_ptr)(void);
typedef user_point_t* user_struct_ptr;
typedef const volatile int* cv_int_ptr;

/* TYPE_ARRAY: Arrays of different types and dimensions */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef user_point_t* pointer_array[15];
typedef int (*func_ptr_array[8])(void);

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*simple_callback)(void);
typedef void (*void_callback)(int, float, char*);
typedef struct simple_struct* (*struct_return_callback)(user_point_t);
typedef int (*complex_callback)(int (*)(float), void**, const char*);
typedef union complex_union (*union_callback)(int_array_1d, struct_ptr);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are recognized by gengtype based on naming patterns */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Transparent union attribute for GCC */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    float* float_ptr;
    void* void_ptr;
} transparent_union_t;

/* Complex nested type hierarchy */
typedef struct nested_container {
    /* Array of pointers to unions */
    union complex_union* union_array[5];
    
    /* Pointer to array of structs */
    struct complex_struct (*struct_matrix)[3][3];
    
    /* Callback that returns pointer to struct with callback */
    struct callback_holder* (*get_callback_holder)(void);
    
    /* Multi-dimensional array of various types */
    int multi_array[2][3][4][5];
} nested_container_t;

/* Struct containing callback members */
struct callback_holder {
    simple_callback cb1;
    complex_callback cb2;
    void_callback cb3;
    nested_container_t container;
};

/* Function pointer returning pointer to struct containing callbacks */
typedef struct callback_holder* (*meta_callback)(complex_callback, nested_container_t*);

/* Ultimate complex nested type */
typedef struct ultimate_type {
    meta_callback getter;
    nested_container_t* containers[10];
    transparent_union_t transparent;
    struct tree_node* gcc_internal;  /* TYPE_LANG_STRUCT reference */
    volatile const int special_flag;
} __attribute__((aligned(32))) ultimate_type_t;

/* Additional GCC-specific attributed types */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
};

union __attribute__((aligned(64))) aligned_union {
    long double ld;
    char buffer[64];
};

/* Inline function using the types */
static inline void dummy_reference(void) {
    /* Reference various types to ensure they're not optimized away */
    volatile int dummy = sizeof(struct undefined_struct) + 
                        sizeof(scalar_int) +
                        sizeof(string_ptr) +
                        sizeof(struct simple_struct) +
                        sizeof(user_point_t) +
                        sizeof(union simple_union) +
                        sizeof(int_ptr) +
                        sizeof(int_array_1d) +
                        sizeof(simple_callback) +
                        sizeof(struct tree_common);
    (void)dummy;
}

#endif /* TEST_TYPES_H */
