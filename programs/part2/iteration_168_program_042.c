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
const char *global_string = "test_string";
char global_string_array[] = "array_string";
const char *const global_const_string = "const_string";

/* TYPE_STRUCT: Complete struct types with mixed members */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double *dbl_ptr;
    int array[10];
    struct complex_struct *next;
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

struct aligned_struct {
    long long data;
    char padding;
} __attribute__((aligned(64)));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    int z;
} point_3d_t;

typedef struct complex_struct complex_t;
typedef struct packed_struct packed_t;

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
    long long raw_data;
};

union transparent_union {
    int *int_ptr;
    float *float_ptr;
    void *void_ptr;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float **float_ptr_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
void (*func_ptr)(void);
const volatile char *cv_ptr;

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
point_3d_t point_array[3][3];
int *pointer_array[50];
void (*callback_array[10])(int, char*);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(void);
typedef void (*void_callback_t)(int, float, char*);
typedef struct simple_struct* (*struct_callback_t)(int id, const char *name);
typedef union simple_union (*union_callback_t)(int, double);
typedef void (*complex_callback_t)(int (*nested)(float), char **strings);

/* Complex nested type definitions */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    union simple_union *union_ptr_array[20];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_holder* (*get_callback_holder)(int);
    
    /* Multi-dimensional array of structs */
    point_3d_t points[4][4];
    
    /* Pointer to array of function pointers */
    int_callback_t (*callback_ptr_array)[10];
} container_t;

struct callback_holder {
    void_callback_t callback;
    struct callback_holder *next;
    int data;
};

/* TYPE_LANG_STRUCT: GCC internal structure (pattern recognized by gengtype) */
struct tree_node;
struct tree_common;
struct tree_type;

/* Even more complex nesting for deep traversal */
typedef struct ultra_complex {
    /* Array of pointers to unions containing structs */
    union {
        struct simple_struct simple;
        struct complex_struct complex;
        container_t container;
    } *variant_array[8];
    
    /* Function pointer with complex signature */
    union simple_union (*processor)(
        int mode,
        struct ultra_complex *self,
        void (*progress)(int percent),
        char *buffer[]
    );
    
    /* Nested struct with callback array */
    struct {
        int_callback_t callbacks[5];
        void (*cleanup)(struct ultra_complex*);
    } handlers;
    
    /* Pointer to pointer to callback */
    void_callback_t **callback_pptr;
} ultra_complex_t;

/* Global instances for reference */
extern struct simple_struct global_simple;
extern union simple_union global_union;
extern container_t global_container;
extern ultra_complex_t *global_ultra_complex;

#endif /* TEST_TYPES_H */
