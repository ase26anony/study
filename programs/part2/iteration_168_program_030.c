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
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int counter;
    float ratio;
    char label[20];
} user_struct_t;

typedef struct complex_struct complex_t;

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union nested_union {
    struct {
        int type;
        union {
            int int_val;
            double dbl_val;
            char *str_val;
        } data;
    } tagged;
    long long raw;
};

/* TYPE_POINTER: Various pointer declarations */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
user_struct_t *user_struct_ptr;
void *void_ptr;
const volatile int *cv_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[50][20];
struct simple_struct struct_array[10];
user_struct_t user_struct_array[5][3];
int *pointer_array[25];
char *string_array[] = {"one", "two", "three"};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, float, char*);
typedef struct simple_struct* (*struct_returning_callback)(int param);
typedef void (*nested_callback)(int (*inner_callback)(float));

/* Complex nested type with function pointers */
struct callback_container {
    simple_callback cb1;
    complex_callback cb2;
    int (*direct_cb)(double, char);
};

/* TYPE_LANG_STRUCT: GCC internal structure (pattern match) */
struct tree_node;
struct tree_common;
struct tree_type;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    int flags;
};

/* Complex nested type hierarchy */
typedef struct node {
    int value;
    struct node *next;
    struct node *prev;
    void (*print)(struct node*);
} node_t;

typedef struct container {
    node_t *nodes[100];
    union {
        int as_int;
        node_t *as_node;
    } storage;
    struct callback_container callbacks;
} container_t;

/* Function pointer returning pointer to struct containing callbacks */
typedef container_t* (*container_factory)(int size, simple_callback init);

/* Struct containing array of pointers to unions */
struct union_container {
    union simple_union *union_ptrs[50];
    union nested_union nested_unions[10];
};

/* Ultimate nested type example */
typedef struct ultimate_type {
    container_t *main_container;
    struct union_container *union_cont;
    container_factory factory;
    nested_callback recursive_cb;
    struct ultimate_type *self_ref;
} ultimate_t;

#endif /* TEST_TYPES_H */
