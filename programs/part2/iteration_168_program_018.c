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

/* TYPE_STRING: String literals and pointers */
const char *global_string_ptr = "Hello, World!";
char global_string_array[] = "Test String";
const char *const global_const_string_ptr = "Constant String";

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
    int width;
    int height;
    double aspect_ratio;
} rectangle_t;

typedef struct complex_struct complex_t;

/* TYPE_UNION: Union definitions */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union tagged_union {
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            char *string_value;
        } data;
    } tagged;
    long raw_data;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
rectangle_t *typedef_ptr;
void *void_ptr;
char **double_ptr;
int (*func_ptr)(void);

/* TYPE_ARRAY: Arrays of different types */
int int_array[10];
float float_array[5][5];
struct simple_struct struct_array[20];
rectangle_t typedef_array[3][3];
char *pointer_array[15];
int (*func_ptr_array[5])(int, int);
union simple_union union_array[8];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(int, int);
typedef void (*void_callback_t)(void *data);
typedef char *(*string_callback_t)(const char *input);
typedef struct simple_struct (*struct_callback_t)(int param);
typedef void (*complex_callback_t)(int a, float b, char *c, void *d);

/* Complex nested structure with callbacks */
struct processor {
    int_callback_t process_int;
    string_callback_t process_string;
    void_callback_t cleanup;
    void *user_data;
};

/* TYPE_LANG_STRUCT: GCC internal structure (pattern recognized by gengtype) */
struct tree_node;
struct tree_common;
struct tree_type;

/* More complex type nesting */
typedef struct node {
    int value;
    struct node *next;
    struct node *prev;
    void (*print)(struct node *);
} node_t;

typedef union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            node_t *node_val;
            void (*callback)(int);
        } value;
    } variant;
    unsigned char raw[16];
} variant_t;

/* Struct containing array of pointers to unions */
struct container {
    int count;
    variant_t *variants[10];
    int_callback_t validators[5];
    char *names[20];
};

/* Function pointer returning pointer to struct containing callback */
typedef struct processor *(*processor_factory_t)(int_callback_t, string_callback_t);

/* Transparent union for GCC attribute testing */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
} transparent_union_t;

/* Aligned struct */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

/* Nested type hierarchy */
typedef struct base {
    int type;
    void (*virtual_func)(struct base *);
} base_t;

typedef struct derived {
    base_t base;
    int extra_data;
    char *name;
} derived_t;

typedef struct more_derived {
    derived_t derived;
    float precision;
    int_callback_t calculator;
} more_derived_t;

/* Array of function pointers with different signatures */
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} operation_t;

typedef double (*arithmetic_func_t)(double, double);

/* Complete type coverage structure */
struct type_coverage {
    /* SCALAR */
    int scalar_member;
    float float_member;
    
    /* STRING */
    const char *string_member;
    char string_array[64];
    
    /* STRUCT */
    struct simple_struct nested_struct;
    
    /* USER_STRUCT */
    rectangle_t user_struct_member;
    
    /* UNION */
    union simple_union union_member;
    
    /* POINTER */
    void *pointer_member;
    struct container *container_ptr;
    
    /* ARRAY */
    int int_member_array[8];
    rectangle_t rect_array[4];
    
    /* CALLBACK */
    int_callback_t callback_member;
    
    /* LANG_STRUCT (forward declared) */
    struct tree_node *tree_node_ptr;
    
    /* UNDEFINED */
    struct undefined_struct *undefined_ptr;
};

#endif /* TEST_TYPES_H */
