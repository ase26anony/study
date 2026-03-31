#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef unsigned int scalar_uint;
typedef _Bool scalar_bool;

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct types */
struct simple_struct {
    int id;
    float value;
    char name[32];
};

struct complex_struct {
    scalar_int counter;
    scalar_double data[10];
    struct simple_struct nested;
    void* generic_ptr;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    char label[20];
} user_point_t;

typedef struct user_node {
    int data;
    struct user_node *next;
    user_point_t position;
} user_node_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

typedef union {
    long long_value;
    double double_value;
    struct simple_struct struct_value;
} user_union_t;

/* GCC-specific union attribute */
union transparent_union_example {
    int *int_ptr;
    long *long_ptr;
} __attribute__((transparent_union));

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef user_point_t* user_struct_ptr_t;
typedef union simple_union* union_ptr_t;
typedef void (*func_ptr_t)(void);
typedef const void* const_void_ptr_t;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10[10];
typedef struct simple_struct struct_array_5[5];
typedef user_point_t* pointer_array_8[8];
typedef int multi_dim_array[3][4][5];
typedef const char* string_array[4];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback_t)(int, float);
typedef void (*void_callback_t)(struct complex_struct*, user_point_t);
typedef user_node_t* (*node_callback_t)(int, const char*);
typedef double (*complex_callback_t)(int_array_10, struct_ptr_t, void_callback_t);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are typically from GCC's internal tree representation */
struct tree_common;
struct tree_type;
struct tree_decl;

/* Complex nested type definitions for deep traversal */

/* Struct containing array of pointers to unions */
struct container_with_union_ptrs {
    int id;
    union_ptr_t union_ptrs[8];
    int_array_10 counters;
};

/* Function pointer returning pointer to struct containing callback */
typedef struct callback_container* (*factory_callback_t)(int);
struct callback_container {
    int id;
    void_callback_t callback;
    factory_callback_t factory;
    struct callback_container *next;
};

/* Typedef for complex nested type hierarchy */
typedef struct {
    struct container_with_union_ptrs base;
    complex_callback_t processor;
    user_union_t variant;
    string_ptr description;
} complex_nested_t;

/* More GCC-specific attributes */
struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[4];
    int flags;
};

/* Mixed attribute struct */
typedef struct __attribute__((packed, aligned(8))) {
    unsigned char type;
    unsigned int length;
    void* data;
} network_packet_t;

/* Array of function pointers */
typedef int (*operation_funcs_t[5])(int, int);

/* Union with struct and array */
union mixed_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
            char str_val[16];
        } data;
    } tagged;
    unsigned char raw[24];
};

/* Forward declaration that will be defined later */
struct forward_declared;
typedef struct forward_declared* forward_ptr_t;

struct forward_declared {
    int magic;
    forward_ptr_t self_ptr;
    void (*method)(struct forward_declared*);
};

/* Enum types (treated as scalars by gengtype) */
typedef enum {
    STATE_INIT,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_STOPPED
} system_state_t;

typedef enum color {
    COLOR_RED = 0xFF0000,
    COLOR_GREEN = 0x00FF00,
    COLOR_BLUE = 0x0000FF
} color_t;

/* Const qualified types */
typedef const int const_int_t;
typedef const struct simple_struct const_struct_t;
typedef int* const const_ptr_t;

/* Volatile qualified types */
typedef volatile int volatile_int_t;
typedef volatile float* volatile_float_ptr_t;

/* Restrict qualified pointers */
typedef int* __restrict__ restrict_int_ptr_t;

/* Complete the undefined struct from earlier */
struct undefined_struct {
    int defined_now;
    undefined_ptr_t chain;
};

#endif /* TEST_TYPES_H */
