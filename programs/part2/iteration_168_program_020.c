#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
void undefined_function_prototype(struct undefined_struct*);

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int scalar_int;
float scalar_float;
double scalar_double;
char scalar_char;
long scalar_long;
short scalar_short;
unsigned int scalar_unsigned;
_Bool scalar_bool;

/* ==================== TYPE_STRING ==================== */
/* String literals and string pointers */
const char* string_pointer = "test_string";
char string_array[] = "another_string";
const char* const string_const_pointer = "const_string";

/* ==================== TYPE_STRUCT ==================== */
/* Complete struct types with mixed members */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    double* dbl_ptr;
    int array[10];
    struct complex_struct* next;
};

struct nested_struct {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } data;
    char tag;
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
    struct node* left;
    struct node* right;
} tree_node_t;

typedef struct complex_struct complex_struct_alias;

/* ==================== TYPE_UNION ==================== */
/* Union types with various members */
union simple_union {
    int i;
    float f;
    char c;
    void* ptr;
};

union complex_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char* str_val;
        } data;
    } tagged;
    long long raw;
};

/* GCC-specific transparent union */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    float* float_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Pointers to all previously defined types */
struct simple_struct* struct_ptr;
vector3d_t* user_struct_ptr;
union simple_union* union_ptr;
int* scalar_ptr;
char** string_ptr_ptr;
void* void_ptr;
const volatile int* cv_ptr;

/* Function pointer declaration (more in TYPE_CALLBACK) */
typedef int (*comparator_func)(const void*, const void*);
comparator_func comp_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of different dimensions and element types */
int scalar_array[100];
float float_array[10][20];
struct simple_struct struct_array[5];
vector3d_t user_struct_array[3][3];
union simple_union union_array[8];
int* pointer_array[15];
char* string_array_array[4][10];

/* Multi-dimensional array */
int multi_dim_array[2][3][4][5];

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */
typedef int (*int_callback)(void);
typedef void (*void_callback)(int, float);
typedef struct simple_struct* (*struct_callback)(int, char*);
typedef void (*complex_callback)(int (*)(float), void*);

/* Callback with nested function pointer parameter */
typedef int (*nested_callback)(int (*filter)(int), void* context);

/* Callback returning pointer to struct containing callback */
typedef struct callback_container* (*meta_callback)(void);
struct callback_container {
    int_callback cb1;
    void_callback cb2;
    void* data;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;

/* Dummy struct with tree-like structure mimicking GCC internals */
struct gcc_internal_node {
    int code;
    union {
        long int_val;
        double real_val;
        char* string_val;
        struct gcc_internal_node* node_ptr;
    } u;
    struct gcc_internal_node* chain;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */
typedef struct {
    union complex_union data;
    struct {
        int count;
        vector3d_t* items;
        void (*processor)(vector3d_t*, int);
    } container;
    int (*validator)(struct complex_struct*, union simple_union*);
} super_complex_t;

/* Struct containing array of pointers to unions */
struct union_container {
    int id;
    union simple_union* union_ptrs[10];
    complex_callback callback;
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_wrapper* (*callback_factory)(int);
struct callback_wrapper {
    nested_callback main_cb;
    void* user_data;
    struct callback_wrapper* next;
};

/* ==================== GCC ATTRIBUTES ==================== */
/* Types with GCC-specific attributes */
struct __attribute__((aligned(16), packed)) aligned_packed_struct {
    char c;
    int i;
    double d;
} __attribute__((aligned(16)));

union __attribute__((packed)) packed_union {
    char data[5];
    int value;
};

typedef struct __attribute__((may_alias)) aliasing_struct {
    int x;
    float y;
} aliasing_struct_t;

/* ==================== FINAL COMPLEX TYPE ==================== */
/* Ultimate nested type covering multiple categories */
typedef struct ultimate_type {
    /* TYPE_SCALAR */
    int type_tag;
    
    /* TYPE_UNION */
    union {
        /* TYPE_STRUCT */
        struct {
            /* TYPE_ARRAY of TYPE_POINTER to TYPE_USER_STRUCT */
            vector3d_t* points[10];
            /* TYPE_CALLBACK */
            int (*compare)(vector3d_t*, vector3d_t*);
        } vector_data;
        
        /* TYPE_STRING */
        char* string_data;
        
        /* TYPE_ARRAY of TYPE_ARRAY */
        int matrix[4][4];
    } data;
    
    /* TYPE_POINTER to TYPE_LANG_STRUCT (forward declared) */
    struct tree_node* ast_node;
    
    /* TYPE_CALLBACK with complex signature */
    void (*traversal_cb)(struct ultimate_type*, 
                        void (*visit)(struct ultimate_type*, void*), 
                        void* context);
    
    /* TYPE_POINTER to self (recursive) */
    struct ultimate_type* next;
} ultimate_type_t;

#endif /* TEST_TYPES_H */
