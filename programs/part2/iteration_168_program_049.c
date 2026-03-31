#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined;
struct another_undefined;
void undefined_function(void *param); /* void pointer parameter */

/* TYPE_SCALAR: Fundamental types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* TYPE_STRING: String types */
const char *global_string = "Hello, World!";
char string_array[] = "Test String";
const char *const constant_string = "Constant";

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int x;
    float y;
    char z;
};

struct complex_struct {
    int id;
    char name[50];
    struct simple_struct nested;
    void *data;
};

struct packed_struct {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
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

/* TYPE_UNION: Union definitions */
union simple_union {
    int i;
    float f;
    char c;
};

union complex_union {
    struct simple_struct s;
    point_t p;
    double d;
    void *ptr;
};

typedef union {
    long long_value;
    double double_value;
    const char *string_value;
} variant_t;

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Array definitions */
int int_array[10];
float float_array[5][5];
struct simple_struct struct_array[20];
point_t point_array[100];
int *pointer_array[50];
char char_2d_array[3][4];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback)(int, int);
typedef void (*void_callback)(void);
typedef char *(*string_callback)(const char *);
typedef struct simple_struct (*struct_callback)(int);
typedef void (*complex_callback)(int, float, const char *, void *);

/* Complex nested type with callback */
struct callback_container {
    int id;
    int_callback func;
    void *user_data;
};

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;

/* More complex nested types to ensure deep traversal */

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union simple_union *items[10];
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*factory_callback)(int, const char *);

/* Struct with nested arrays and pointers */
struct deeply_nested {
    int matrix[3][3];
    struct simple_struct *structs[5];
    union complex_union unions[2];
    int_callback callbacks[4];
};

/* Transparent union for GCC attribute */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    long *long_ptr;
} transparent_union_t;

/* Aligned struct */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

/* Typedef for complex nested type hierarchy */
typedef struct {
    struct deeply_nested nested;
    factory_callback create;
    variant_t variant;
    transparent_union_t transparent;
} mega_struct_t;

/* Final complex type that references everything */
typedef struct {
    /* Scalars */
    int type_id;
    float priority;
    
    /* Strings */
    const char *name;
    
    /* Structs */
    struct simple_struct basic;
    complex_t complex;
    
    /* Unions */
    union complex_union data;
    
    /* Pointers */
    void **ptr_array;
    
    /* Arrays */
    int matrix[4][4];
    
    /* Callbacks */
    complex_callback handler;
    
    /* Nested user struct */
    point_t position;
    
    /* Reference to lang struct */
    struct tree_node *tree_ref;
} ultimate_type_t;

#endif /* TEST_TYPES_H */
