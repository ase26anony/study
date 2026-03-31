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
    char c;
} __attribute__((packed));

struct aligned_struct {
    long long data;
    char padding;
} __attribute__((aligned(64)));

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int counter;
    float ratio;
    char label[20];
} user_defined_t;

typedef struct complex_struct complex_t;
typedef struct packed_struct packed_t;

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
            double double_value;
            char *string_value;
        } data;
    } tagged;
    long long raw_data;
};

union transparent_union_example {
    int *int_ptr;
    float *float_ptr;
    void *void_ptr;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer declarations */
int *int_ptr;
float **float_ptr_ptr;
struct simple_struct *struct_ptr;
union simple_union *union_ptr;
user_defined_t *user_struct_ptr;
void *void_ptr;
const volatile int *cv_int_ptr;
char *const const_char_ptr = "Read-only pointer";

/* TYPE_ARRAY: Arrays of different dimensions and types */
int int_array[100];
float float_2d_array[10][20];
struct simple_struct struct_array[50];
union simple_union union_array[25];
user_defined_t user_array[30];
int *pointer_array[40];
const char *string_array[] = {"one", "two", "three", NULL};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(int, float, char*);
typedef struct simple_struct* (*struct_returning_callback_t)(int id);
typedef int (*array_processing_callback_t)(int array[], size_t length);
typedef void (*void_callback_t)(void);

/* Function pointer variables */
simple_callback_t global_callback;
complex_callback_t process_data;

/* TYPE_LANG_STRUCT: GCC internal structure (dummy forward declaration) */
struct tree_node;
struct tree_common;
struct tree_type;
struct tree_decl;

/* Complex nested type hierarchy */
typedef struct container {
    int id;
    union {
        struct {
            int type;
            void *data;
        } generic;
        struct {
            float x, y, z;
        } coordinates;
        char buffer[256];
    } payload;
    
    /* Array of function pointers */
    simple_callback_t callbacks[10];
    
    /* Pointer to array of structs */
    struct simple_struct (*struct_matrix_ptr)[5][5];
    
    /* Nested struct with union */
    struct {
        int tag;
        union {
            int int_val;
            double dbl_val;
            struct container *next;
        } value;
    } node;
} container_t;

/* Even more complex nested types */
typedef union mega_union {
    container_t container;
    struct {
        int magic;
        /* Pointer to function returning pointer to struct */
        container_t* (*allocator)(size_t);
        /* Array of pointers to functions */
        void (*operations[8])(container_t*);
    } manager;
    long long raw[16];
} mega_union_t;

/* Struct containing all type categories */
struct type_kitchen_sink {
    /* SCALAR */
    int scalar_member;
    
    /* STRING */
    const char *string_member;
    
    /* STRUCT (nested) */
    struct simple_struct nested_struct;
    
    /* USER_STRUCT */
    user_defined_t user_struct;
    
    /* UNION */
    union simple_union data_union;
    
    /* POINTER */
    void *data_ptr;
    int (*func_ptr)(int);
    
    /* ARRAY */
    double values[8];
    container_t *objects[4];
    
    /* CALLBACK */
    complex_callback_t handler;
    
    /* LANG_STRUCT pointer */
    struct tree_node *tree_node_ptr;
    
    /* UNDEFINED pointer */
    struct undefined_struct *undefined_ptr;
} __attribute__((aligned(32)));

/* Transparent union for system-like interfaces */
typedef union {
    int fd;
    void *handle;
    long long token;
} resource_t __attribute__((transparent_union));

/* Function pointer with complex return type */
typedef struct type_kitchen_sink* (*sink_factory_t)(
    int config,
    const char *name,
    resource_t resource,
    void (*cleanup)(struct type_kitchen_sink*)
);

/* Global instance with complex initialization */
struct type_kitchen_sink global_sink = {
    .scalar_member = 42,
    .string_member = "Initialized",
    .nested_struct = {1, 2.0f, 'A'},
    .user_struct = {100, 3.14f, "Label"},
    .data_union = {.as_int = 0xDEADBEEF},
    .data_ptr = &global_sink,
    .func_ptr = NULL,
    .values = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8},
    .objects = {NULL, NULL, NULL, NULL},
    .handler = NULL,
    .tree_node_ptr = NULL,
    .undefined_ptr = NULL
};

#endif /* TEST_TYPES_H */
