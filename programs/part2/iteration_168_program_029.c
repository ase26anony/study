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
_Bool global_bool;
long global_long;
long long global_long_long;
short global_short;
signed char global_signed_char;
unsigned int global_unsigned_int;

/* TYPE_STRING: String literals and string pointers */
const char *global_string_ptr = "Hello, World!";
char global_string_array[] = "Test String";
const char *const global_const_string_ptr = "Constant String";

/* TYPE_STRUCT: Complete struct types with mixed members */
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

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

struct aligned_struct {
    int x;
    double y;
} __attribute__((aligned(64)));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct node {
    int data;
    struct node *left;
    struct node *right;
} tree_node_t;

typedef struct container {
    point_t position;
    tree_node_t *root;
    char tag[16];
} container_t;

/* TYPE_UNION: Union types */
union simple_union {
    int int_val;
    float float_val;
    char char_val;
    void *ptr_val;
};

union complex_union {
    struct simple_struct s;
    struct complex_struct *c;
    point_t p;
    double d;
};

typedef union {
    long long as_ll;
    double as_double;
    void *as_ptr;
} generic_union_t;

/* Transparent union (GCC-specific) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    long *long_ptr;
    void *void_ptr;
} transparent_union_t;

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char *char_ptr;
void *void_ptr;
const void *const_void_ptr;
volatile int *volatile_int_ptr;

struct simple_struct *struct_ptr;
union simple_union *union_ptr;
point_t *user_struct_ptr;
container_t **double_ptr_to_user_struct;

int **pointer_to_int_ptr;
void ***triple_void_ptr;

/* TYPE_ARRAY: Arrays of various types and dimensions */
int int_array[10];
float float_array[5][5];
double double_3d_array[3][3][3];
char char_array[] = {'a', 'b', 'c', '\0'};

struct simple_struct struct_array[5];
point_t user_struct_array[10][10];
union simple_union union_array[8];

int *pointer_array[20];
void *void_pointer_array[15];
struct complex_struct *struct_pointer_array[7][7];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(void);
typedef void (*void_callback_t)(int, float);
typedef char *(*string_callback_t)(const char *, int);
typedef struct simple_struct *(*struct_callback_t)(int, void *);
typedef void (*complex_callback_t)(int (*)(float), void **);

/* Function pointers with different signatures */
int (*func_ptr_int)(int, int);
void (*func_ptr_void)(void);
double (*func_ptr_double)(double *, int);
int (*func_ptr_array)(int[], int);

/* Callback in struct */
struct callback_container {
    int_callback_t int_cb;
    string_callback_t string_cb;
    void (*custom_cb)(struct callback_container *);
};

/* TYPE_LANG_STRUCT: GCC internal structures */
/* These are recognized by gengtype as language-specific structures */
struct tree_common;
struct tree_decl_common;
struct tree_type_common;

/* Dummy structures with GCC internal naming patterns */
struct gcc_internal_struct {
    int code;
    union {
        long int_val;
        double real_val;
        void *ptr_val;
    } u;
};

/* Complex nested type hierarchy */
typedef struct nested_container {
    /* Struct containing array of pointers to unions */
    generic_union_t *union_ptr_array[5];
    
    /* Function pointer returning pointer to struct */
    struct nested_container *(*get_next)(struct nested_container *);
    
    /* Callback member */
    int (*process)(struct nested_container *, void *);
    
    /* Nested struct */
    struct {
        int depth;
        point_t coordinates[4];
        void *data;
    } inner;
    
    /* Pointer to callback */
    complex_callback_t complex_cb;
    
    /* Array of structs containing function pointers */
    struct {
        int id;
        int (*operation)(int, int);
        void (*cleanup)(void);
    } operations[3];
    
} nested_container_t;

/* Even more complex: function pointer returning pointer to struct containing callbacks */
typedef nested_container_t *(*factory_t)(int, const char *);

/* Transparent union with function pointers */
typedef union __attribute__((transparent_union)) {
    int (*int_func)(int);
    float (*float_func)(float);
    void (*void_func)(void);
} func_union_t;

/* Variable with GCC attribute */
int __attribute__((aligned(16), used)) aligned_var = 42;

/* Const pointer to array of function pointers */
int (* const const_func_ptr_array[5])(void);

/* Volatile struct with bitfields */
struct volatile_struct {
    volatile unsigned int flags : 4;
    volatile int counter : 28;
    volatile char status;
};

#endif /* TEST_TYPES_H */
