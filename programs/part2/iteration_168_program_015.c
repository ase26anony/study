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
unsigned int global_uint;

/* TYPE_STRING: String literals and string pointers */
const char *global_string = "Hello, World!";
char global_string_array[] = "Test String";
const char *const global_const_string = "Constant String";

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

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    int z;
} point_3d_t;

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
    double raw_data[2];
};

/* TYPE_POINTER: Various pointer declarations */
int *int_ptr;
float **float_ptr_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
point_3d_t *typedef_ptr;
void *void_ptr;
const volatile int *cv_ptr;

/* TYPE_ARRAY: Arrays of different types and dimensions */
int int_array[100];
float float_2d_array[10][20];
struct simple_struct struct_array[5];
point_3d_t typedef_array[3][3];
int *pointer_array[50];
void *void_ptr_array[25];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct simple_struct*, point_3d_t*, int);
typedef char *(*string_callback_t)(const char*, int);
typedef void (*void_callback_t)(void);

/* Complex nested type with function pointers */
struct callback_container {
    simple_callback_t math_op;
    complex_callback_t process_data;
    string_callback_t get_string;
    void_callback_t cleanup;
};

/* TYPE_LANG_STRUCT: GCC internal structure (tree_node) */
struct tree_node;
struct tree_node *tree_root;

/* GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
};

/* Complex nested type hierarchy */
typedef struct node {
    int value;
    struct node *left;
    struct node *right;
    void (*traverse)(struct node*);
} tree_node_t;

/* Array of pointers to unions */
union simple_union *union_ptr_array[20];

/* Struct containing array of pointers to unions */
struct union_container {
    int count;
    union simple_union *items[10];
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*factory_callback_t)(int, const char*);

/* Even more complex nesting */
struct ultimate_nest {
    /* Scalar */
    int id;
    
    /* String */
    char name[64];
    
    /* Struct */
    struct simple_struct base;
    
    /* User struct */
    point_3d_t position;
    
    /* Union */
    union nested_union data;
    
    /* Pointer */
    struct ultimate_nest *next;
    
    /* Array */
    int matrix[4][4];
    
    /* Array of pointers */
    void *handlers[8];
    
    /* Callback */
    factory_callback_t create;
    
    /* Pointer to lang struct */
    struct tree_node *ast_node;
    
    /* Nested struct with attribute */
    struct __attribute__((packed)) {
        unsigned char flags;
        unsigned int size;
    } metadata;
};

/* Global instance to ensure types are used */
extern struct ultimate_nest global_nested_instance;

#endif /* TEST_TYPES_H */
