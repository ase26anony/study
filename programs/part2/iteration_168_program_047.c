#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct undefined_struct;
union undefined_union;
typedef struct undefined_struct *undefined_ptr_t;

/* TYPE_SCALAR: Fundamental types */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef char scalar_char;
typedef long scalar_long;
typedef unsigned int scalar_uint;

/* TYPE_STRING: String types */
typedef const char* string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    int id;
    float value;
    char name[16];
};

struct complex_struct {
    scalar_int counter;
    scalar_float data[10];
    struct simple_struct nested;
    string_ptr description;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    int z;
} user_struct_t;

typedef struct tagged_struct {
    user_struct_t coordinates;
    double timestamp;
    struct tagged_struct *next;
} tagged_struct_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

typedef union {
    struct {
        int type;
        union simple_union data;
    } typed;
    struct {
        long long combined;
    } raw;
} complex_union_t;

/* GCC-specific union attribute */
union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    const void *void_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr_t;
typedef struct simple_struct* struct_ptr_t;
typedef union simple_union* union_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef const char** string_ptr_ptr;

/* Pointer to incomplete type */
typedef struct undefined_struct* undefined_struct_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[8];
typedef int_ptr_t pointer_array[20];
typedef char* string_array_array[4][16];

/* Multi-dimensional complex array */
typedef union complex_union_t complex_union_array[3][3];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*int_callback_t)(int, float);
typedef void (*void_callback_t)(struct simple_struct*, const char*);
typedef user_struct_t* (*struct_creator_t)(int, int, int);
typedef int (*array_processor_t)(int[], size_t);
typedef void (*complex_callback_t)(int_callback_t, void_callback_t);

/* Nested callback type */
typedef void (*callback_register_t)(complex_callback_t);

/* TYPE_LANG_STRUCT: GCC internal-like structure */
/* This mimics GCC's internal tree structure naming pattern */
struct tree_common {
    int code;
    union tree_common *chain;
    union tree_common *type;
};

struct tree_int_cst {
    struct tree_common common;
    long long int_cst;
};

/* Forward declaration that might be recognized */
struct tree_node;

/* Complex nested type combining multiple categories */
typedef struct container {
    /* Scalar members */
    scalar_int id;
    scalar_float priority;
    
    /* String member */
    string_ptr name;
    
    /* Struct member */
    user_struct_t position;
    
    /* Union member */
    union simple_union data;
    
    /* Pointer members */
    struct container *next;
    void *user_data;
    int_callback_t processor;
    
    /* Array members */
    int values[5];
    struct simple_struct items[3];
    void_func_ptr_t handlers[4];
    
    /* Nested complex type */
    struct {
        int depth;
        struct container *parent;
        complex_callback_t notify;
    } metadata;
    
    /* Attribute for alignment */
    long long __attribute__((aligned(16))) aligned_data;
} container_t;

/* Function pointer returning pointer to struct with callback */
typedef container_t* (*container_factory_t)(
    int id,
    const char* name,
    int_callback_t processor
);

/* Union containing array of function pointers */
union callback_container {
    void (*void_handlers[5])(void);
    int (*int_handlers[3])(int);
    complex_callback_t complex_handler;
};

/* Packed struct with bitfields */
struct __attribute__((packed)) packed_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 24;
    char code;
};

/* Array of pointers to unions containing structs */
typedef union {
    struct simple_struct simple;
    user_struct_t user;
    struct packed_struct packed;
} variant_t;

typedef variant_t* variant_ptr_array[10];

/* Final complex typedef nesting all categories */
typedef struct {
    container_t main;
    variant_ptr_array variants;
    callback_register_t registrar;
    struct tree_common *tree_node;
    undefined_ptr_t undefined_ref;
} master_type_t;

#endif /* TEST_TYPES_H */
