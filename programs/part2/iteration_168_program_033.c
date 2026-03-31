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

/* TYPE_STRING: String literals and string pointers */
const char *global_string = "Hello, World!";
char string_array[] = "Test String";
const char *string_pointers[] = {"str1", "str2", "str3"};

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
        union simple_union data;
    } tagged;
    double raw_value;
};

/* TYPE_POINTER: Various pointer declarations */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
void *void_ptr;
user_struct_t *user_struct_ptr;
int **double_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[50][20];
struct simple_struct struct_array[10];
user_struct_t user_array[5][3];
int *pointer_array[25];
char char_matrix[4][4][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(int, float);
typedef void (*void_callback_t)(void);
typedef struct simple_struct *(*struct_callback_t)(int, char*);
typedef void (*complex_callback_t)(int (*)(float), void*);

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node) */
struct tree_node;
typedef struct tree_node *tree;
struct tree_common {
    tree chain;
    tree type;
    int uid;
};

/* Complex nested type definitions */
typedef struct container {
    int id;
    union nested_union data;
    int_callback_t callback;
    struct simple_struct elements[5];
    void **pointer_table;
    struct container *next;
} container_t;

/* Function pointer returning pointer to struct with callback */
typedef container_t *(*factory_callback_t)(int, void_callback_t);

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union simple_union *items[10];
    factory_callback_t factory;
};

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
    double data[8];
    char padding;
};

/* Complex nested hierarchy */
typedef struct node {
    int value;
    struct node *left;
    struct node *right;
    void (*visit)(struct node*);
} tree_node_t;

/* Array of function pointers */
typedef int (*operation_t)(int, int);
operation_t operations[10];

/* Struct with nested anonymous struct */
struct outer {
    struct {
        int x;
        int y;
    } coord;
    struct inner {
        float data;
        struct outer *parent;
    } nested;
};

#endif /* TEST_TYPES_H */
