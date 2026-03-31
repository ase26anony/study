#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct *undefined_ptr_t;

/* Void in pointer contexts without full definition */
extern void *global_void_ptr;
typedef void (*void_func_ptr)(void);

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
char str_array[] = "test_string";
const char *const_string_ptr = "constant_string";
char *mutable_string = "mutable";

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
    double *data_ptr;
    int array[10];
    struct complex_struct *next;
};

struct nested_members {
    int scalar_field;
    struct {
        int inner_x;
        float inner_y;
    } anonymous;
    union {
        int as_int;
        float as_float;
    } value_union;
};

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_field;
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
    struct node *left;
    struct node *right;
} tree_node_t;

/* ==================== TYPE_UNION ==================== */
/* Union types with various members */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
    void *ptr_val;
};

union variant {
    struct {
        int type;
        union data_union data;
    } tagged;
    long long raw;
};

/* Transparent union (GCC-specific) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    float *float_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to all previously defined types */
struct simple_struct *simple_ptr;
complex_t *complex_ptr;
point_t *point_ptr;
union data_union *union_ptr;
tree_node_t **double_ptr_ptr;

/* Function pointers */
int (*func_ptr)(int, float);
void (*void_func)(void);

/* Void pointer */
void *generic_ptr;

/* Pointer to array */
int (*array_ptr)[10];

/* Pointer to pointer */
void **void_double_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_array[20][30];
double multi_dim_array[5][10][15];

/* Array of structs */
struct simple_struct struct_array[50];
point_t point_array[25];

/* Array of pointers */
int *pointer_array[40];
void *void_ptr_array[30];

/* Array of arrays */
int matrix[3][4];

/* Array of function pointers */
int (*func_ptr_array[10])(int);

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */

/* Simple callback */
typedef int (*simple_callback)(void);

/* Callback with parameters */
typedef void (*param_callback)(int a, float b, const char *msg);

/* Callback returning pointer */
typedef struct simple_struct *(*struct_return_callback)(int id);

/* Callback taking callback as parameter */
typedef int (*higher_order_callback)(int (*inner)(int), int value);

/* Callback with variable arguments */
typedef int (*varargs_callback)(const char *fmt, ...);

/* Complex nested callback */
typedef void (*complex_callback)(struct complex_struct *cs, 
                                 union data_union *du,
                                 point_t (*transform)(point_t));

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;

/* Dummy structs with GCC internal naming patterns */
struct lang_type {
    struct tree_node *base;
    void *lang_specific;
};

struct lang_decl {
    struct tree_node *base;
    int lang_flags;
};

/* Struct that might be recognized by gengtype */
struct GTY(()) gcc_internal_struct {
    struct tree_node *node;
    struct lang_type *type_info;
    union {
        int as_int;
        void *as_ptr;
    } u;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */

/* Struct containing array of pointers to unions */
struct container_of_unions {
    int count;
    union data_union *union_ptrs[20];
    struct container_of_unions *next;
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*meta_callback)(int);
struct callback_container {
    int id;
    simple_callback cb;
    param_callback param_cb;
    meta_callback self_referential;
};

/* Typedef for complex nested type hierarchy */
typedef struct {
    struct {
        point_t position;
        complex_t *data;
        union variant current;
    } state;
    
    struct callback_container *callbacks[5];
    
    union {
        int mode;
        struct {
            unsigned int flags;
            void *user_data;
        } advanced;
    } config;
} system_state_t;

/* ==================== GCC-SPECIFIC ATTRIBUTES ==================== */
/* Apply various GCC attributes to influence gengtype categorization */

struct __attribute__((aligned(16), packed)) aligned_packed_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(32)));

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
};

typedef struct __attribute__((transparent_union)) {
    int *ptr;
    const void *cptr;
} transparent_wrapper_t;

/* Struct with section attribute */
struct __attribute__((section(".special_section"))) section_struct {
    int magic;
    void *handler;
};

/* ==================== FINAL COMPLEX EXAMPLE ==================== */
/* Ultimate nested type combining everything */

typedef struct ultimate_type {
    /* Scalar */
    int version;
    
    /* String */
    const char *name;
    
    /* Struct */
    struct simple_struct base;
    
    /* User struct */
    point_t position;
    
    /* Union */
    union variant data;
    
    /* Pointer */
    struct ultimate_type *next;
    
    /* Array */
    complex_callback callbacks[8];
    
    /* Array of structs */
    struct callback_container containers[4];
    
    /* Nested anonymous struct */
    struct {
        unsigned long flags;
        transparent_union_t tu;
    } attributes;
    
    /* Flexible array member */
    int dynamic_data[];
} ultimate_type_t;

/* Function pointer operating on the ultimate type */
typedef ultimate_type_t *(*ultimate_creator)(int, const char *, complex_callback);

#endif /* TEST_TYPES_H */
