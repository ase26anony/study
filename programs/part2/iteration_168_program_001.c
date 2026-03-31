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
    scalar_float data[10];
    struct simple_struct nested;
    char *description;
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    double z;
} user_struct_t;

typedef struct tagged_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        char *str_val;
    } data;
} tagged_struct_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    scalar_double as_double[4];
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*void_func_ptr)(void);
typedef const volatile char* special_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef user_struct_t* pointer_array[15];
typedef char string_array[3][50];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback)(int, float);
typedef void (*void_callback)(struct simple_struct*, user_struct_t*);
typedef char* (*string_callback)(const char*, int);
typedef double (*complex_callback)(int_array_10, float_array_2d*, void_callback);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These are patterns that gengtype might recognize from GCC's internal representation */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Complex nested type definitions with GCC attributes */
struct __attribute__((aligned(16), packed)) aligned_struct {
    int id;
    char data;
    double value;
} __attribute__((packed));

union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    float* float_ptr;
    void* void_ptr;
};

/* More complex nested types for deep traversal */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    union complex_union* union_ptrs[8];
    
    /* Function pointer returning pointer to struct */
    struct simple_struct* (*get_struct)(int id);
    
    /* Callback member */
    complex_callback processor;
    
    /* Multi-dimensional array */
    float matrix[3][3][3];
    
    /* Pointer to array of callbacks */
    void_callback (*callback_array)[5];
} container_t;

/* Even more complex hierarchy */
typedef struct top_level {
    container_t containers[4];
    struct top_level* next;
    struct top_level* prev;
    
    /* Union with struct containing callback */
    union {
        struct {
            int_callback int_handler;
            string_callback string_handler;
        } handlers;
        struct {
            void* data;
            size_t size;
        } buffer;
    } variant;
    
    /* Pointer to function returning pointer to function */
    int (*(*complex_func_ptr)(int))(float);
} top_level_t;

/* Function pointer type with nested struct parameter */
typedef void (*nested_callback)(struct {
    int depth;
    top_level_t* levels;
    void (*traverse)(int);
} config_t);

/* Final complex typedef for maximum coverage */
typedef struct ultimate_type {
    /* Mix of all type categories */
    scalar_int base;
    string_ptr name;
    user_struct_t user;
    union simple_union variant;
    int_array_10 counts;
    struct simple_struct* refs[10];
    int_callback validator;
    struct tree_node* lang_node;  /* TYPE_LANG_STRUCT reference */
    
    /* Nested anonymous struct */
    struct {
        nested_callback walker;
        container_t* (*factory)(int, float);
    } operations;
    
    /* Bitfield for completeness */
    unsigned int flags : 4;
    unsigned int mode : 2;
} ultimate_type_t;

#endif /* TEST_TYPES_H */
