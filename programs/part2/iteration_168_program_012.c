#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct undefined_t;

/* Void in pointer context without full definition */
extern void *undefined_void_ptr;
extern const void *const_undefined_void_ptr;

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* Scalar parameters in function declarations */
extern int scalar_func(int param_int, float param_float, double param_double);

/* ==================== TYPE_STRING ==================== */
/* String literals in initializations */
const char global_string[] = "global test string";
static char static_string[] = "static string";
char mutable_string[] = "mutable string";

/* String pointers */
const char *string_ptr = "pointer to string literal";
char *mutable_string_ptr = mutable_string;

/* Array of strings */
const char *string_array[] = {"first", "second", "third"};

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct types with mixed members */
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

struct nested_struct {
    struct {
        int x;
        int y;
    } point;
    struct {
        float width;
        float height;
    } dimensions;
};

/* Struct with GCC attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    int flags;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedefs for struct types */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct complex_struct complex_t;

typedef struct {
    point_t position;
    float velocity;
    char name[64];
} particle_t;

/* Typedef with attributes */
typedef struct __attribute__((packed)) {
    unsigned char type;
    unsigned int length;
    char data[1];
} packet_t;

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union simple_union {
    int as_int;
    float as_float;
    char as_char[4];
};

/* Union with struct members */
union data_union {
    struct {
        int type;
        int value;
    } integer;
    struct {
        int type;
        double value;
    } floating;
    struct {
        int type;
        char text[256];
    } string;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    long *long_ptr;
    void *void_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to previously defined types */
struct simple_struct *simple_struct_ptr;
complex_t *complex_t_ptr;
point_t *point_ptr;
union simple_union *union_ptr;

/* Void pointers */
void *generic_ptr;
const void *const_generic_ptr;
volatile void *volatile_generic_ptr;

/* Pointer to pointer */
int **int_ptr_ptr;
struct simple_struct ***struct_ptr_ptr_ptr;

/* Function pointers (also TYPE_CALLBACK) */
typedef int (*int_func_ptr)(int);
typedef void (*void_func_ptr)(void);
typedef struct simple_struct* (*struct_creator_ptr)(int, float);

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_2d_array[10][20];
double double_3d_array[5][5][5];

/* Struct arrays */
struct simple_struct struct_array[50];
point_t point_array[25][25];

/* Pointer arrays */
void *pointer_array[100];
int_func_ptr func_ptr_array[10];

/* Flexible array member in struct */
struct flexible_array_struct {
    int count;
    double data[];
};

/* Zero-length array (GCC extension) */
struct zero_length_array {
    int id;
    char extra[0];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */

/* Simple callback */
typedef void (*simple_callback)(void);

/* Callback with parameters */
typedef int (*int_callback)(int a, int b);
typedef float (*float_callback)(float x, float y, float z);

/* Callback returning pointer */
typedef char* (*string_callback)(const char *input);
typedef struct simple_struct* (*struct_callback)(int id);

/* Callback taking callback as parameter */
typedef void (*callback_of_callback)(int_callback cb);

/* Callback with complex parameters */
typedef void (*complex_callback)(
    struct simple_struct *s,
    point_t *points,
    int count,
    simple_callback completion
);

/* Callback returning callback */
typedef simple_callback (*callback_retriever)(int mode);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
/* These names are recognized by gengtype as language-specific */

struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy struct with tree_ prefix pattern */
struct tree_dummy {
    int code;
    union {
        long intval;
        double realval;
        char *strval;
    } u;
    struct tree_dummy *chain;
};

/* Another GCC internal-like structure */
struct gimple_statement_d;
typedef struct gimple_statement_d *gimple;

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union data_union *unions[10];
    union simple_union (*union_getter)(int index);
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container* (*factory_func)(void);

struct callback_container {
    int id;
    simple_callback on_start;
    complex_callback on_data;
    callback_of_callback on_complete;
    factory_func clone;
};

/* Typedef for complex nested type */
typedef struct {
    struct {
        point_t position;
        particle_t *particles;
        int particle_count;
    } system;
    
    union {
        struct {
            int_callback int_handler;
            string_callback string_handler;
        } handlers;
        struct {
            void *data;
            size_t size;
        } raw;
    } config;
    
    struct callback_container *(*container_factory)(int, const char*);
    
    /* Multi-dimensional array of function pointers */
    simple_callback event_matrix[5][5];
} super_complex_t;

/* Even more nesting */
typedef struct outermost {
    super_complex_t inner;
    struct {
        struct union_container *uc;
        struct callback_container **cc_array;
        int depth;
    } nested;
    
    /* Array of pointers to arrays of structs */
    point_t *(*get_point_matrix(void))[10][10];
    
    /* Callback that returns a pointer to a function that takes a callback */
    callback_of_callback (*meta_callback)(int mode);
} outermost_t;

#endif /* TEST_TYPES_H */
