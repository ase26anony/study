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

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct definitions */
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

/* TYPE_USER_STRUCT: Typedefs for struct types */
typedef struct {
    int x;
    int y;
    double z;
} user_struct_t;

typedef struct complex_struct complex_struct_alias;

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
        } data;
    } tagged;
    char raw_data[16];
};

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*void_func_ptr)(void);
typedef const volatile char* special_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_1d[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef int* pointer_array[15];
typedef const char* string_array[];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback)(int, float);
typedef void (*void_callback)(struct simple_struct*, union simple_union*);
typedef char* (*string_callback)(const char*, int);
typedef struct complex_struct* (*struct_ret_callback)(void);
typedef void (*complex_callback)(int (*)(float), void*);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Using patterns that might be recognized by gengtype */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Complex nested type definitions with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    int data[4];
    char padding[12];
};

struct __attribute__((packed)) packed_struct {
    char a;
    int b;
    short c;
    double d;
};

union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    void* void_ptr;
    char* char_ptr;
};

/* More complex nested types for deep traversal */
typedef struct nested_container {
    /* Struct containing array of pointers to unions */
    union tagged_union* union_ptrs[8];
    
    /* Pointer to function returning pointer to struct */
    struct complex_struct* (*get_complex)(int);
    
    /* Callback member */
    int_callback processor;
    
    /* Multi-dimensional array */
    int matrix[3][3][3];
    
    /* Nested struct with attributes */
    struct __attribute__((packed)) {
        unsigned char flags;
        unsigned int value;
    } metadata;
} nested_container_t;

/* Function pointer returning pointer to struct containing callback */
typedef nested_container_t* (*container_factory)(void_callback, int);

/* Even more complex type hierarchy */
typedef struct ultimate_type {
    container_factory factory;
    nested_container_t* containers[5];
    union {
        int (*int_funcs[3])(int);
        void (*void_funcs[3])(void);
    } func_union;
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2;
        unsigned int flag3 : 5;
    } bits;
    
    /* Flexible array member */
    int flexible_array[];
} ultimate_type_t;

/* Template for generating multiple similar types */
#define DECLARE_SPECIALIZED_STRUCT(name, type) \
    struct name##_struct { \
        type data; \
        void (*process)(type); \
        struct name##_struct* next; \
    }

DECLARE_SPECIALIZED_STRUCT(int_specialized, int);
DECLARE_SPECIALIZED_STRUCT(float_specialized, float);
DECLARE_SPECIALIZED_STRUCT(ptr_specialized, void*);

/* External declarations to create more undefined types */
extern struct external_undefined;
extern union external_undefined_union;

#endif /* TEST_TYPES_H */
