#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* TYPE_STRING: String types and literals */
const char *global_string = "Hello, World!";
char global_string_array[] = "Test String";
const char *const global_const_string = "Constant String";

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

struct nested_struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int int_val;
        float float_val;
    } data;
    char tag;
};

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct simple_struct simple_t;
typedef struct complex_struct complex_t;
typedef struct nested_struct nested_t;

typedef struct {
    int counter;
    char buffer[256];
    void *user_data;
} anonymous_struct_t;

/* TYPE_UNION: Union types */
union basic_union {
    int int_member;
    float float_member;
    double double_member;
    char *string_member;
};

union complex_union {
    struct simple_struct struct_member;
    union basic_union nested_union;
    void *pointer_member;
    int array_member[4];
};

/* GCC-specific union attribute */
union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    float *float_ptr;
    void *void_ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
double **double_ptr_ptr;
struct simple_struct *struct_ptr;
union basic_union *union_ptr;
void *void_ptr;
const volatile char *cv_ptr;

/* Function pointer typedef (will be used in TYPE_CALLBACK) */
typedef int (*comparator_t)(const void *, const void *);

/* TYPE_ARRAY: Arrays of different types and dimensions */
int int_array[10];
float float_array[5][5];
double double_3d_array[3][3][3];
struct simple_struct struct_array[20];
union basic_union union_array[15];
int *pointer_array[8];
char string_array[4][64];

/* Packed struct with GCC attribute */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    double c;
    char d;
};

/* Aligned struct with GCC attribute */
struct __attribute__((aligned(32))) aligned_struct {
    int data[8];
    double precision;
};

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef void (*simple_callback_t)(void);
typedef int (*int_callback_t)(int, int);
typedef char *(*string_callback_t)(const char *, int);
typedef struct simple_struct *(*struct_callback_t)(int id, const char *name);
typedef void (*complex_callback_t)(int, float, double, const char *, void *);

/* Struct containing callback members */
struct callback_container {
    simple_callback_t init;
    int_callback_t process;
    string_callback_t get_name;
    complex_callback_t cleanup;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are forward declarations that match GCC's internal naming patterns */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Dummy struct that might be recognized by gengtype */
struct lang_type {
    struct tree_node *base;
    void *lang_specific;
};

struct lang_decl {
    struct tree_node *base;
    unsigned lang_flag : 1;
};

/* Complex nested type hierarchy */
typedef struct node {
    int value;
    struct node **children;  /* Array of pointers */
    int num_children;
    union {
        int int_data;
        double double_data;
        char *string_data;
    } payload;
    void (*visit)(struct node *);  /* Callback member */
} node_t;

/* Even more complex: struct containing array of pointers to unions */
struct container_of_unions {
    int count;
    union complex_union *items[10];  /* Array of pointers to unions */
    comparator_t sort_func;  /* Callback function pointer */
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container *(*factory_t)(int type);

/* Mixed attribute struct */
struct __attribute__((packed, aligned(8))) mixed_attr_struct {
    char flag;
    int value;
    double data;
    void (*operation)(struct mixed_attr_struct *);
};

/* Transparent union parameter in function pointer */
typedef int (*transparent_func_t)(union transparent_union_t);

/* Volatile and const qualified types */
volatile int volatile_int;
const double const_double = 3.14159;
volatile const char *volatile_const_string;

/* Anonymous union within struct */
struct with_anonymous_union {
    int type;
    union {
        int int_value;
        float float_value;
        void *pointer_value;
    };
    char name[20];
};

/* Bitfield struct */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int reserved : 24;
    int normal_field;
};

#endif /* TEST_TYPES_H */
