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
long long global_long_long;

/* TYPE_STRING: String literals and string pointers */
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
    double values[10];
    struct simple_struct nested;
    char name[50];
};

struct packed_struct {
    char a;
    int b;
    double c;
} __attribute__((packed));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int counter;
    float ratio;
    char label[20];
} user_struct_t;

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
    long long raw;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
float *float_ptr;
double *double_ptr;
char **string_ptr_ptr;
void *void_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
user_struct_t *user_struct_ptr;
int (*function_ptr)(void);
void (*callback_ptr)(int, char*);

/* TYPE_ARRAY: Arrays of different types */
int int_array[100];
float float_array[10][20];
double multi_dim_array[5][10][15];
struct simple_struct struct_array[50];
union simple_union union_array[25];
user_struct_t user_struct_array[30];
int *pointer_array[40];
void *void_ptr_array[35];
int (*func_ptr_array[10])(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(void);
typedef void (*void_callback_t)(int, float, char*);
typedef struct simple_struct* (*struct_callback_t)(int, const char*);
typedef union simple_union (*union_callback_t)(double, int*);
typedef user_struct_t* (*complex_callback_t)(int, float, char**, void*);

/* Complex nested callback example */
typedef void (*nested_callback_t)(int (*)(float), struct simple_struct*);

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Complex nested type hierarchy */
typedef struct container {
    int id;
    
    /* Array of pointers to unions */
    union simple_union *union_ptrs[20];
    
    /* Function pointer returning pointer to struct with callback */
    struct callback_container* (*get_callback_container)(int);
    
    /* Nested struct with array of function pointers */
    struct {
        int size;
        void (*handlers[10])(void*);
    } handler_block;
    
    /* Pointer to array of structs */
    user_struct_t (*user_struct_matrix)[10];
    
    /* Complex callback member */
    complex_callback_t processor;
} container_t;

/* Struct containing callback that returns pointer to struct with callbacks */
struct callback_container {
    int state;
    nested_callback_t notify;
    struct {
        int_callback_t get_value;
        void_callback_t set_value;
    } operations;
    
    /* Pointer to another container */
    container_t *next;
};

/* Even more complex nested type */
typedef struct ultimate_nest {
    /* Array of pointers to callback containers */
    struct callback_container *containers[5];
    
    /* Matrix of function pointers */
    int (*operations[3][3])(int, int);
    
    /* Union containing struct with array of pointers */
    union {
        struct {
            int count;
            void *data_ptrs[100];
        } data_block;
        long long signature;
    } storage;
    
    /* Callback that takes callback as parameter */
    void (*event_handler)(void (*callback)(int), container_t*);
} ultimate_nest_t;

/* GCC attribute examples */
struct aligned_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(64)));

struct transparent_union_struct {
    union {
        int *int_ptr;
        float *float_ptr;
        char *char_ptr;
    } u;
} __attribute__((transparent_union));

/* Mixed attribute usage */
typedef struct __attribute__((packed)) attribute_mix {
    unsigned char type;
    int data __attribute__((aligned(8)));
    void (*func)(void) __attribute__((noreturn));
} attribute_mix_t;

#endif /* TEST_TYPES_H */
