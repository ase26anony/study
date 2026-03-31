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

/* TYPE_STRING: String literals and string pointers */
const char *global_string = "Hello, World!";
char string_array[] = "Test String";
const char *string_pointers[] = {"str1", "str2", "str3"};

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    char label[20];
} point_t;

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
    } variant;
    long long raw_data;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
point_t *user_struct_ptr;
void *void_ptr;
int **double_ptr;
const volatile int *cv_ptr;

/* TYPE_ARRAY: Arrays of different dimensions and types */
int int_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
point_t user_struct_array[3][4];
int *pointer_array[8];
int (*array_of_pointers)[10];
int multi_dim_array[2][3][4][5];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(int, int);
typedef void (*void_callback)(void);
typedef char *(*string_callback)(const char *);
typedef struct simple_struct *(*struct_callback)(int id);
typedef int (*complex_callback)(int (*)(int), void *);

/* Complex function pointer with nested types */
typedef union simple_union *(*union_callback)(
    point_t *points, 
    int count,
    void (*progress)(int)
);

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
};

struct __attribute__((aligned(32))) aligned_struct {
    double data[4];
    long counter;
};

/* Complex nested type hierarchy */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    struct {
        union simple_union *item_ptrs[10];
        int count;
    } union_container;
    
    /* Function pointer returning pointer to struct with callback */
    struct nested_struct *(*get_nested)(int id, simple_callback cb);
    
    /* Multi-dimensional array of complex types */
    point_t grid[5][5];
    
    /* Pointer to callback */
    complex_callback processor;
} container_t;

struct nested_struct {
    container_t *parent;
    simple_callback handlers[3];
    union nested_union data;
};

/* More complex nesting examples */
typedef struct node {
    struct node *children[4];
    void *data;
    int (*compare)(struct node *, struct node *);
    union {
        int int_val;
        struct node *node_ptr;
        void (*func_ptr)(void);
    } variant;
} node_t;

/* Array of function pointers */
typedef int (*operation_funcs[5])(int, int);

/* Struct with all type categories combined */
struct master_type {
    /* SCALAR */
    int scalar_member;
    
    /* STRING */
    const char *string_member;
    
    /* STRUCT */
    struct simple_struct nested_struct;
    
    /* USER_STRUCT */
    point_t user_struct_member;
    
    /* UNION */
    union simple_union union_member;
    
    /* POINTER */
    void *pointer_member;
    
    /* ARRAY */
    int array_member[5];
    
    /* CALLBACK */
    simple_callback callback_member;
    
    /* POINTER to ARRAY */
    int (*ptr_to_array)[10];
    
    /* ARRAY of CALLBACKS */
    void_callback callbacks[3];
    
    /* Nested complex type */
    container_t *container_ptr;
};

#endif /* TEST_TYPES_H */
