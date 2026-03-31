#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declarations of incomplete structs */
struct undefined_struct;
struct another_undefined;
void undefined_function(void* ptr); /* void pointer parameter */

/* ==================== TYPE_SCALAR ==================== */
/* Fundamental scalar types */
int global_int;
float global_float;
double global_double;
char global_char;
long global_long;
short global_short;
unsigned int global_uint;
_Bool global_bool;

/* Function with scalar parameters */
int scalar_function(int a, float b, double c, char d);

/* ==================== TYPE_STRING ==================== */
/* String literals and string pointers */
const char* global_string = "Hello, World!";
char string_array[] = "Test String";
const char* const_string_array[] = {"str1", "str2", "str3"};

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
    struct complex_struct* next; /* Self-referential pointer */
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

typedef struct complex_struct complex_t;

typedef struct {
    vector3d_t position;
    vector3d_t velocity;
    float mass;
} particle_t;

/* ==================== TYPE_UNION ==================== */
/* Union types */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char* string_val;
};

union variant {
    struct {
        int type;
        union data_union data;
    } tagged;
    long long raw;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    float* float_ptr;
    void* void_ptr;
} transparent_union_t;

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
void* void_pointer;
int* int_pointer;
float* float_pointer;
double* double_pointer;
char** string_pointer_pointer;

/* Pointers to previously defined types */
struct simple_struct* simple_struct_ptr;
complex_t* complex_ptr;
vector3d_t* vector_ptr;
union data_union* union_ptr;

/* Function pointers (more in TYPE_CALLBACK section) */
int (*func_ptr)(int, int);

/* Pointer to array */
int (*array_ptr)[10];

/* ==================== TYPE_ARRAY ==================== */
/* Arrays of various types and dimensions */
int scalar_array[100];
float float_array[50][20];
double double_3d_array[10][10][10];

/* Arrays of structs and unions */
struct simple_struct struct_array[5];
vector3d_t vector_array[100];
union data_union union_array[20];

/* Array of pointers */
int* pointer_array[30];
struct simple_struct* struct_ptr_array[15];
void* void_ptr_array[25];

/* String array (already covered in TYPE_STRING) */

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types with various signatures */

/* Simple callback */
typedef int (*int_callback)(int);

/* Callback returning pointer */
typedef void* (*alloc_callback)(size_t);

/* Callback with multiple parameters */
typedef double (*math_callback)(double, double, int);

/* Callback taking callback as parameter */
typedef int (*higher_order_callback)(int_callback, int);

/* Struct with callback members */
struct callback_container {
    int_callback on_start;
    alloc_callback on_alloc;
    void (*on_finish)(void*);
};

/* Callback returning struct pointer */
typedef struct simple_struct* (*struct_factory)(int, float);

/* Complex nested callback type */
typedef int (*(*callback_factory)(int))(float, double);

/* ==================== TYPE_LANG_STRUCT ==================== */
/* GCC internal language-specific structures */
struct tree_node;
struct tree_common;
struct tree_type;

/* Dummy structs that might be recognized by gengtype */
struct __attribute__((gcc_internal)) gcc_internal_struct {
    int code;
    union {
        long int_val;
        double real_val;
        struct tree_node* tree_ptr;
    } u;
};

/* ==================== COMPLEX TYPE NESTING ==================== */
/* Deeply nested type hierarchy */

/* Struct containing array of pointers to unions */
struct nested_container {
    union data_union* union_ptrs[10];
    struct {
        int count;
        void** items;
    } dynamic_array;
};

/* Function pointer returning pointer to struct containing callbacks */
typedef struct callback_container* (*container_factory)(void);

/* Typedef for complex nested type */
typedef struct {
    struct nested_container containers[5];
    container_factory factory;
    math_callback calculators[3];
    union variant variants[8];
} mega_struct_t;

/* Even more complex: pointer to array of function pointers */
typedef int (*(*complex_array_ptr)[10])(int, int);

/* Struct with all types combined */
struct ultimate_type {
    /* SCALAR */
    int scalar_member;
    
    /* STRING */
    const char* name;
    
    /* STRUCT */
    struct simple_struct simple;
    
    /* USER_STRUCT */
    vector3d_t vector;
    
    /* UNION */
    union data_union data;
    
    /* POINTER */
    void* generic_ptr;
    struct ultimate_type* self_ptr;
    
    /* ARRAY */
    int int_array[20];
    struct simple_struct struct_array[5];
    
    /* CALLBACK */
    int_callback callback;
    
    /* Nested complex type */
    mega_struct_t mega;
    
    /* Array of function pointers */
    math_callback callbacks[5];
    
    /* Pointer to array of structs */
    vector3d_t (*vector_matrix)[10][10];
};

/* ==================== GCC ATTRIBUTES ==================== */
/* Various GCC attributes applied to types */

struct __attribute__((packed)) tightly_packed {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(64))) cache_aligned {
    double data[8];
};

union __attribute__((transparent_union)) another_transparent_union {
    int* ip;
    const char** cpp;
};

/* Function with attributes */
void __attribute__((noinline)) 
attributed_function(struct cache_aligned* aligned_ptr) 
    __attribute__((nonnull(1)));

#endif /* TEST_TYPES_H */
