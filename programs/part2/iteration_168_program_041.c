#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined;
struct another_undefined;
typedef struct undefined* undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;

/* TYPE_STRING: String literals and string pointers */
const char* global_string = "Hello, World!";
char global_array_string[] = "Array String";
static const char* static_string = "Static String";

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    double values[10];
    struct simple_struct* nested;
    char name[50];
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int data;
    float precision;
} user_t;

typedef struct complex_struct complex_t;

typedef struct {
    user_t user;
    complex_t* complex;
} nested_user_t;

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union tagged_union {
    struct {
        int type;
        union simple_union data;
    } tagged;
    double raw_data;
};

typedef union {
    int i;
    float f;
    double d;
} numeric_union_t;

/* TYPE_POINTER: Various pointer types */
int* int_ptr;
float* float_ptr;
double* double_ptr;
char* char_ptr;
void* void_ptr;
struct simple_struct* struct_ptr;
union simple_union* union_ptr;
user_t* user_struct_ptr;
int** double_int_ptr;
void (*function_ptr)(void);

/* TYPE_ARRAY: Arrays of various types and dimensions */
int int_array[10];
float float_array[5][5];
double double_3d_array[3][3][3];
char char_array[] = {'a', 'b', 'c', '\0'};
struct simple_struct struct_array[20];
user_t user_array[15];
int* pointer_array[8];
int (*array_of_function_ptrs[5])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(int, float);
typedef void (*void_callback_t)(void*);
typedef struct simple_struct* (*struct_callback_t)(int, char*);
typedef union simple_union (*union_callback_t)(double);

/* Complex callback with nested types */
typedef user_t* (*complex_callback_t)(int_callback_t, struct complex_struct*);

/* TYPE_LANG_STRUCT: GCC internal structure (dummy declaration) */
struct tree_node;
struct tree_common;
typedef struct tree_node* tree;

/* GCC-specific attributed structures */
struct __attribute__((aligned(16))) aligned_struct {
    int data;
    double value;
};

struct __attribute__((packed, aligned(4))) packed_aligned_struct {
    char flag;
    int counter;
    short index;
};

/* Transparent union for GCC attribute */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* void_ptr;
} transparent_union_t;

/* Nested type hierarchy for deep traversal */
struct deeply_nested {
    int id;
    struct {
        union {
            int as_int;
            float as_float;
        } value;
        struct deeply_nested* next;
    } inner;
    int (*processor)(struct deeply_nested*, void*);
    user_t users[5];
};

/* Array of pointers to unions containing callbacks */
union callback_container {
    int_callback_t int_func;
    void_callback_t void_func;
    struct_callback_t struct_func;
};

union callback_container callback_array[10];

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_struct* (*meta_callback_t)(int);
struct callback_struct {
    int id;
    meta_callback_t generator;
    int_callback_t processor;
};

/* Final complex typedef with everything */
typedef struct {
    struct undefined* undefined_ptr;      /* TYPE_UNDEFINED */
    int scalar_field;                     /* TYPE_SCALAR */
    const char* string_field;             /* TYPE_STRING */
    struct complex_struct struct_field;   /* TYPE_STRUCT */
    user_t user_field;                    /* TYPE_USER_STRUCT */
    union simple_union union_field;       /* TYPE_UNION */
    void** pointer_field;                 /* TYPE_POINTER */
    int array_field[7][7];                /* TYPE_ARRAY */
    complex_callback_t callback_field;    /* TYPE_CALLBACK */
    struct tree_node* lang_struct_field;  /* TYPE_LANG_STRUCT */
} ultimate_type_t;

#endif /* TEST_TYPES_H */
