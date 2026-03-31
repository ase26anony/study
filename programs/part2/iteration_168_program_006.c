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

/* TYPE_STRING: String types */
const char *global_string = "Hello, World!";
char string_array[] = "Test String";
const char *const constant_string = "Constant";

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

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    char label[20];
} point_t;

typedef struct complex_struct complex_t;

/* TYPE_UNION: Union types */
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
    double raw[2];
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
void *void_ptr;
char **string_ptr_ptr;
const volatile int *cv_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[50][20];
struct simple_struct struct_array[10];
point_t point_array[5][5];
int *pointer_array[25];
void *void_ptr_array[10];

/* Multi-dimensional arrays */
int matrix[3][3][3];
char char_grid[10][20];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback)(int, int);
typedef void (*void_callback)(void);
typedef struct simple_struct *(*struct_factory)(int id);
typedef int (*array_processor)(int array[], size_t length);
typedef void (*complex_callback)(struct complex_struct *, point_t *, int (*)(int));

/* Function pointer variables */
simple_callback math_operation;
void_callback cleanup_handler;
struct_factory create_struct;

/* Nested callback in struct */
struct callback_container {
    simple_callback func;
    void *user_data;
    int (*validator)(const char *);
};

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;

/* Complex nested type hierarchy */
typedef struct nested_container {
    /* Struct containing array of pointers to unions */
    union simple_union *union_array[8];
    
    /* Pointer to function returning pointer to struct with callback */
    struct callback_container *(*get_callback)(int id);
    
    /* Multi-dimensional array of structs */
    point_t grid[4][4];
    
    /* Pointer to array of function pointers */
    simple_callback (*callbacks)[5];
    
    /* Self-referential pointer */
    struct nested_container *parent;
    
    /* Array of pointers to different types */
    void *heterogeneous[10];
} nested_container_t;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    int flags;
};

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
};

/* Typedef for complex nested type */
typedef struct {
    nested_container_t container;
    struct callback_container callbacks[3];
    union nested_union storage;
    int (*processor)(nested_container_t *, point_t [][4]);
} super_complex_t;

/* More function pointer variations */
typedef int (*variadic_callback)(int, ...);
typedef void (*const_callback)(const struct simple_struct *);
typedef volatile int *(*ptr_generator)(void);

/* Mixed declarations with attributes */
extern const volatile int cv_global __attribute__((used));
static inline int inline_func(int x) __attribute__((always_inline));

#endif /* TEST_TYPES_H */
