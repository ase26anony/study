#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct undefined_t;

/* Void in pointer contexts without full definition */
extern void *undefined_pointer;
typedef void (*undefined_callback)(void);

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
long global_long = 100L;
unsigned int global_uint = 100U;
_Bool global_bool = 1;

/* Function parameters with scalar types */
extern int process_scalars(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals in initializations */
char global_string[] = "Hello, World!";
const char *const_string_ptr = "Constant string";
char *string_array[] = {"first", "second", "third"};

/* String in struct */
struct string_container {
    const char *name;
    char buffer[256];
};

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct types with mixed members */
struct simple_struct {
    int id;
    float value;
    char tag;
};

struct complex_struct {
    int counter;
    double data[10];
    struct simple_struct *nested;
    char name[50];
};

struct nested_struct {
    struct {
        int x;
        int y;
    } point;
    struct complex_struct complex;
};

/* Struct with GCC attributes */
struct aligned_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

struct packed_struct {
    short s;
    int i;
    char c;
} __attribute__((packed));

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedefs for struct types */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct complex_struct complex_t;

typedef struct {
    point_t start;
    point_t end;
    double length;
} line_segment_t;

typedef struct {
    int id;
    char *name;
    point_t position;
} entity_t;

/* ==================== TYPE_UNION ==================== */
/* Union types */
union simple_union {
    int i;
    float f;
    char c;
};

union data_union {
    long long_value;
    double double_value;
    void *pointer_value;
    char string_value[32];
};

/* Union with struct members */
union mixed_union {
    struct {
        int type;
        int data;
    } structured;
    double raw;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    long *long_ptr;
    void *void_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
point_t *user_struct_ptr;
void **void_ptr_ptr;

/* Function pointers */
int (*func_ptr)(int, int);
void (*void_func_ptr)(void);

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to pointer */
char **string_ptr_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_array[20][30];
struct simple_struct struct_array[50];
point_t user_struct_array[25][25];
int *pointer_array[40];

/* Multi-dimensional arrays */
int matrix[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
char string_matrix[5][50];

/* Array of function pointers */
int (*func_array[10])(int, int);

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */

/* Simple callback */
typedef int (*int_callback)(int);

/* Callback returning pointer */
typedef struct simple_struct* (*struct_callback)(void);

/* Callback with multiple parameters */
typedef double (*math_callback)(double, double);

/* Callback returning void */
typedef void (*notification_callback)(const char*, int);

/* Callback taking callback as parameter */
typedef int (*higher_order_callback)(int_callback, int);

/* Callback in struct */
struct callback_container {
    int_callback handler;
    notification_callback notifier;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;

/* Dummy structs with GCC internal naming patterns */
struct lang_type {
    int lang_specific;
    void *data;
};

struct lang_decl {
    unsigned lang_flag1 : 1;
    unsigned lang_flag2 : 1;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union data_union *items[20];
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container* (*complex_callback)(int, const char*);

/* Typedef for complex nested type */
typedef struct {
    int id;
    struct {
        point_t coordinates[10];
        complex_t *transform;
        union {
            int mode;
            float factor;
        } config;
    } geometry;
    int_callback validators[5];
    struct union_container *container;
} super_complex_t;

/* Even more complex nesting */
typedef struct node {
    int value;
    struct node *left;
    struct node *right;
    void (*traverse)(struct node*, void (*)(int));
} tree_node_t;

/* Struct with all types combined */
struct ultimate_type {
    /* SCALAR */
    int scalar_member;
    
    /* STRING */
    const char *string_member;
    
    /* STRUCT */
    struct simple_struct nested_struct;
    
    /* USER_STRUCT */
    point_t user_struct_member;
    
    /* UNION */
    union simple_union data_union;
    
    /* POINTER */
    void *generic_pointer;
    int (*function_pointer)(int);
    
    /* ARRAY */
    double data_array[100];
    struct callback_container callbacks[10];
    
    /* CALLBACK */
    math_callback calculator;
    
    /* LANG_STRUCT (pointer to) */
    struct tree_node *tree_node;
    
    /* Nested complex type */
    super_complex_t complex_data;
};

/* Transparent union with callback */
typedef union __attribute__((transparent_union)) {
    int_callback int_handler;
    notification_callback string_handler;
} callback_union_t;

#endif /* TEST_TYPES_H */
