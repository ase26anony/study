#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined;
struct another_undefined;
typedef struct undefined *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;

/* TYPE_STRING: String-related types */
const char *global_string = "Hello, World!";
char string_array[] = "Test string";
const char *const constant_string = "Constant";

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    char name[32];
    float values[10];
    struct simple_struct nested;
    void *data;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int counter;
    double precision;
} user_t;

typedef struct complex_struct complex_t;

/* TYPE_UNION: Union types */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

typedef union {
    long long_value;
    double double_value;
    struct simple_struct struct_value;
} tagged_union_t;

/* GCC-specific union attribute */
union __attribute__((transparent_union)) transparent_union {
    int *int_ptr;
    void *void_ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
char **string_ptr_ptr;
struct simple_struct *struct_ptr;
union basic_union *union_ptr;
user_t *user_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[10];
float float_array[5][5];
char char_array[3][4][5];
struct simple_struct struct_array[8];
union basic_union union_array[6];
int *pointer_array[20];
const char *string_array_array[3];

/* Multi-dimensional complex array */
struct complex_struct complex_array[2][3];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, float, char*);
typedef struct simple_struct *(*struct_returning_callback)(int param);
typedef int (*array_parameter_callback)(int arr[], size_t len);
typedef void (*variadic_callback)(const char *fmt, ...);

/* Struct containing callback members */
struct callback_container {
    simple_callback cb1;
    complex_callback cb2;
    struct_returning_callback cb3;
};

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;

/* Complex nested type definitions */

/* Struct containing array of pointers to unions */
struct nested_container {
    int id;
    union basic_union *union_ptrs[10];
    struct callback_container callbacks;
};

/* Function pointer returning pointer to struct containing callback */
typedef struct nested_container *(*complex_func_ptr)(int, struct simple_struct*);

/* Typedef for complex nested type hierarchy */
typedef struct {
    complex_func_ptr generator;
    struct nested_container *containers[5];
    union {
        int mode;
        float threshold;
    } config;
} super_container_t;

/* GCC attributes on types */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    int flags;
};

/* Mixed complex type example */
typedef struct {
    int type;
    union {
        int int_value;
        float float_value;
        char *string_value;
        struct nested_container *container;
    } data;
    void (*cleanup)(void*);
} variant_t;

/* Array of complex nested types */
variant_t variants[10];

/* Final complex type that references everything */
struct master_type {
    /* Scalars */
    int magic;
    
    /* Strings */
    const char *name;
    
    /* Structs */
    struct simple_struct simple;
    complex_t complex;
    
    /* User structs */
    user_t user;
    
    /* Unions */
    union basic_union basic_union;
    tagged_union_t tagged;
    
    /* Pointers */
    void **pointer_array;
    
    /* Arrays */
    int matrix[4][4];
    
    /* Callbacks */
    simple_callback init;
    complex_callback process;
    
    /* Nested container */
    struct nested_container *nested;
    
    /* Language struct pointer */
    struct tree_node *tree_node;
    
    /* Variant type */
    variant_t variant;
    
    /* Self-referential pointer */
    struct master_type *next;
};

#endif /* TEST_TYPES_H */
