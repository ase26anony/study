#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;

/* Void in pointer contexts without full definition */
typedef void *opaque_handle;
extern void undefined_function(void *param);

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
long global_long = 100L;
short global_short = 10;
_Bool global_bool = 1;

/* Function parameters with scalar types */
extern int process_scalars(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals in initializations */
char simple_string[] = "Hello, World!";
const char *const_string_ptr = "Constant string";
char *dynamic_string = "Dynamic string";

/* Array of strings */
const char *string_array[] = {"first", "second", "third"};

/* Struct with string member */
struct string_holder {
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

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 16;
};

/* Struct with array member */
struct array_struct {
    int matrix[3][3];
    float vector[10];
    struct simple_struct objects[5];
};

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

typedef struct node {
    int data;
    struct node *next;
} node_t;

/* ==================== TYPE_UNION ==================== */
/* Simple union */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* Union with struct members */
union data_container {
    struct {
        int type;
        char name[20];
    } metadata;
    struct {
        double x;
        double y;
        double z;
    } coordinates;
    point_t point;
};

/* Transparent union (GCC-specific) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union basic_union *union_ptr;
point_t *user_struct_ptr;
void (*func_ptr)(void);
char **string_ptr_ptr;

/* Pointer to array */
int (*array_ptr)[10];
float (*matrix_ptr)[5][5];

/* Pointer to pointer chain */
struct complex_struct ***triple_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int int_array[100];
float float_array[10][10];
double multi_dim_array[3][4][5];

/* Array of structs */
struct simple_struct struct_array[20];
point_t point_array[50];

/* Array of pointers */
int *pointer_array[30];
struct complex_struct *struct_ptr_array[15];

/* Array of arrays */
int matrix_of_arrays[5][10][15];

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */

/* Simple callback */
typedef void (*simple_callback)(void);

/* Callback with parameters */
typedef int (*int_callback)(int a, int b);

/* Callback returning pointer */
typedef struct simple_struct *(*struct_ret_callback)(int id);

/* Callback with complex signature */
typedef void (*complex_callback)(int count, const char **strings, 
                                 struct complex_struct *data);

/* Callback taking callback as parameter */
typedef void (*higher_order_callback)(int_callback cb, int x, int y);

/* Struct with callback members */
struct callback_container {
    simple_callback init;
    int_callback process;
    complex_callback cleanup;
    higher_order_callback iterate;
};

/* Union with callback */
union callback_union {
    simple_callback simple;
    complex_callback complex;
    void (*generic)(void);
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy struct with pattern that might be recognized */
struct GTY(()) lang_specific_struct {
    int lang_code;
    void *lang_data;
    struct tree_node *associated_node;
};

/* Another potentially recognized pattern */
struct language_function {
    int magic_number;
    struct tree_node *current_function_decl;
    void *language_specific;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */

/* Struct containing array of pointers to unions */
struct nested_level1 {
    union data_container *containers[10];
    struct nested_level2 *next_level;
};

struct nested_level2 {
    int id;
    struct nested_level3 deeper;
    complex_callback processor;
};

struct nested_level3 {
    point_t points[5];
    int (*compute_matrix)[3][3];
    struct {
        int tag;
        union {
            int as_int;
            float as_float;
            point_t as_point;
        } data;
    } variant;
};

/* Typedef for complex nested type */
typedef struct nested_level1 **(*factory_function)(int count, 
                                                   complex_callback cb);

/* Struct with function pointer returning pointer to struct with callback */
struct ultimate_nest {
    struct_ret_callback getter;
    struct {
        int_callback math_op;
        complex_callback string_op;
    } operations;
    struct nested_level1 ***deep_structure;
};

/* ==================== GCC ATTRIBUTES ==================== */
/* Types with GCC-specific attributes */

struct __attribute__((aligned(16), packed)) aligned_packed_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(32)));

union __attribute__((transparent_union)) another_transparent_union {
    int *i;
    long *l;
    double *d;
};

typedef struct __attribute__((packed)) {
    unsigned char type;
    unsigned int length;
    char data[0];
} variable_length_struct;

/* Function with attributes */
typedef void (*attribute_callback)(void) __attribute__((noinline));

/* ==================== COMPREHENSIVE EXAMPLE ==================== */
/* Putting it all together */

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_STRUCT,
    TYPE_ARRAY
} data_type_t;

struct comprehensive_example {
    data_type_t type;
    
    union {
        int int_value;
        float float_value;
        const char *string_value;
        struct simple_struct *struct_value;
        void *array_value;
    } data;
    
    /* Callback for processing */
    int (*processor)(struct comprehensive_example *self);
    
    /* Array of callbacks */
    void (*handlers[5])(void);
    
    /* Nested structure */
    struct {
        int reference_count;
        struct comprehensive_example **references;
    } metadata;
};

#endif /* TEST_TYPES_H */
