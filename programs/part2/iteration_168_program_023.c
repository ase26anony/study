#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
typedef struct undefined_struct undefined_t;

/* Void in pointer context without full definition */
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
int process_scalars(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals in initializations */
char str1[] = "test_string";
const char *str_ptr = "constant_string";
char str_array[3][20] = {"first", "second", "third"};

/* String in struct */
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

struct nested_members {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Struct with GCC attributes */
struct __attribute__((aligned(16), packed)) aligned_struct {
    char c;
    int i;
    double d;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedefs for struct types */
typedef struct {
    int x;
    int y;
    int z;
} vector3d_t;

typedef struct node {
    int data;
    struct node *next;
} node_t;

typedef struct __attribute__((packed)) packed_user {
    unsigned char flags;
    unsigned int value;
} packed_user_t;

/* ==================== TYPE_UNION ==================== */
/* Union types */
union basic_union {
    int i;
    float f;
    char c;
    void *p;
};

union complex_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char *str_val;
        } data;
    } tagged;
    double raw[2];
};

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *generic_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char **string_ptr_ptr;

struct simple_struct *struct_ptr;
union basic_union *union_ptr;
vector3d_t *user_struct_ptr;

void *generic_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

/* Pointer to pointer */
int **int_ptr_ptr;
struct complex_struct ***complex_ptr_ptr_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_2d[10][20];
double double_3d[5][10][15];

struct simple_struct struct_array[50];
vector3d_t user_struct_array[25][25];

int *pointer_array[30];
void *void_ptr_array[40];

/* Array of function pointers */
int (*func_ptr_array[10])(int, int);

/* Flexible array member */
struct flex_array {
    int count;
    int data[];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*int_callback)(int, int);
typedef void (*void_callback)(void);
typedef char *(*string_callback)(const char *);
typedef struct simple_struct *(*struct_callback)(int id);

/* Complex callback signatures */
typedef int (*complex_callback)(int, float, double, char, void *);
typedef void (*callback_with_callback)(int_callback cb);

/* Function returning function pointer */
int_callback get_callback(int type);

/* Struct with callback members */
struct callback_container {
    int_callback int_handler;
    void_callback void_handler;
    string_callback string_handler;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Dummy structs that might be recognized by gengtype */
struct __attribute__((aligned(8))) lang_specific {
    int lang_code;
    void *lang_data;
    struct tree_node *tree;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */
typedef struct nested_container {
    /* Struct containing array of pointers to unions */
    union basic_union *union_ptr_array[20];
    
    /* Pointer to struct containing callback */
    struct callback_container *cb_container;
    
    /* Multi-dimensional array of structs */
    vector3d_t vectors[10][10];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container *(*get_container)(int id);
    
    /* Nested anonymous struct */
    struct {
        int depth;
        struct nested_container *next_level;
    } nested_info;
} nested_container_t;

/* Typedef for complex nested type */
typedef nested_container_t *(*factory_function)(int, int_callback);

/* Union with nested struct */
union ultimate_union {
    struct {
        factory_function create;
        void (*destroy)(nested_container_t *);
        int_callback processor;
    } operations;
    
    struct {
        int type;
        union {
            int i;
            float f;
            struct simple_struct s;
            nested_container_t *nc;
        } data;
    } variant;
};

/* Final complex type definition */
typedef struct __attribute__((packed)) ultimate_type {
    int magic;
    union ultimate_union content;
    struct callback_container callbacks[5];
    nested_container_t *nested;
    void (*finalizer)(struct ultimate_type *);
} ultimate_type_t;

#endif /* TEST_TYPES_H */
