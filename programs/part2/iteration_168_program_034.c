#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;

/* TYPE_STRING: String literals and pointers */
const char *global_string_ptr = "Hello, World!";
char global_string_array[] = "Test String";
const char *const global_const_string_ptr = "Constant String";

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    double values[10];
    struct simple_struct nested;
    char name[50];
} __attribute__((packed));

struct nested_struct {
    struct {
        int a;
        int b;
    } inner;
    struct complex_struct *complex_ptr;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int data;
    float precision;
} user_defined_t;

typedef struct complex_struct complex_t;
typedef struct nested_struct nested_t;

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char *string_val;
        } data;
    } tagged;
    long long raw;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char **string_ptr_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
user_defined_t *user_struct_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[10][20];
double multi_dim_array[5][10][15];
struct simple_struct struct_array[50];
union simple_union union_array[25];
user_defined_t user_struct_array[30];
int *pointer_array[40];
const char *string_array[] = {"one", "two", "three", NULL};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(int, float, const char*);
typedef struct simple_struct* (*struct_returning_callback_t)(int param);
typedef int (*array_processing_callback_t)(int array[], size_t length);
typedef void (*varargs_callback_t)(const char *fmt, ...);

/* Complex nested callback types */
typedef void (*callback_with_callback_t)(int (*nested)(float), void *context);

/* Struct containing callbacks */
struct callback_container {
    simple_callback_t simple;
    complex_callback_t complex;
    callback_with_callback_t nested_callback;
};

/* TYPE_LANG_STRUCT: GCC internal structures */
/* These are recognized by gengtype as language-specific structures */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Dummy struct with tree_ prefix to potentially trigger TYPE_LANG_STRUCT */
struct tree_dummy {
    int code;
    union {
        int ival;
        float fval;
        const char *sval;
    } u;
};

/* Complex type nesting examples */
typedef struct {
    int count;
    union simple_union *items[100];
    struct callback_container callbacks;
    int (*processor)(struct complex_struct*, user_defined_t[]);
} super_complex_t;

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container* (*meta_callback_t)(
    int param,
    void (*progress)(int),
    const char *name
);

/* Array of pointers to unions inside a struct */
struct container_with_unions {
    int size;
    union tagged_union *union_ptrs[50];
    meta_callback_t factory;
};

/* Transparent union attribute test */
typedef union {
    int *int_ptr;
    float *float_ptr;
    double *double_ptr;
} transparent_union_t __attribute__((transparent_union));

/* Aligned struct */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(64)));

/* Packed struct with bitfields */
struct packed_bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value;
} __attribute__((packed));

/* Forward declaration that will be defined later */
struct forward_declared;

/* Complete definition of forward declared struct */
struct forward_declared {
    struct forward_declared *next;
    struct forward_declared *prev;
    void *data;
};

/* Circular reference types */
typedef struct node node_t;
struct node {
    int value;
    node_t *left;
    node_t *right;
};

/* Anonymous struct/union */
struct anonymous_container {
    struct {
        int x;
        int y;
    } point;
    union {
        int int_view;
        float float_view;
    } value;
};

/* Const and volatile qualified types */
typedef const int const_int_t;
typedef volatile float volatile_float_t;
typedef const volatile double cv_double_t;

/* Function returning complex nested type */
complex_t* create_complex(int id, double values[], const char *name);

/* Callback that takes array of structs and returns pointer to union */
union simple_union* (*array_processor_t)(
    struct simple_struct structs[],
    int count,
    void (*callback)(int index)
);

#endif /* TEST_TYPES_H */
