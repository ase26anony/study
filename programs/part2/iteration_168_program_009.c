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
    scalar_double measurements[10];
    struct simple_struct* nested;
    char description[256];
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    int x;
    int y;
    int z;
} user_struct_t;

typedef struct tagged_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        char* str_val;
    } data;
} tagged_struct_t;

/* TYPE_UNION: Union types */
union simple_union {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
};

union complex_union {
    struct {
        int type;
        char name[16];
    } header;
    struct {
        int x, y;
        double value;
    } point;
    struct {
        char* text;
        int length;
    } string;
};

/* Apply GCC attributes to some unions */
union attributed_union {
    int i;
    char c;
    double d;
} __attribute__((packed, aligned(8)));

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct simple_struct* struct_ptr;
typedef union simple_union* union_ptr;
typedef void (*func_ptr)(void);
typedef const char* const* string_ptr_ptr;
typedef int*** triple_int_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10[10];
typedef float float_array_2d[5][5];
typedef struct simple_struct struct_array[20];
typedef union simple_union union_array[15];
typedef int_ptr pointer_array[8];
typedef char string_array[4][64];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback)(void);
typedef void (*void_callback)(int, float, char*);
typedef struct simple_struct* (*struct_callback)(int id, const char* name);
typedef union simple_union (*union_callback)(double param);
typedef int (*complex_callback)(int (*nested)(float), void* context);

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Using patterns that might be recognized by gengtype */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_exp;

/* Complex nested type definitions */
typedef struct nested_container {
    int id;
    
    /* Array of pointers to unions */
    union complex_union* union_ptrs[5];
    
    /* Function pointer returning pointer to struct */
    struct complex_struct* (*get_complex)(int id);
    
    /* Callback member */
    complex_callback processor;
    
    /* Multi-dimensional array */
    float matrix[3][3][3];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Anonymous struct */
    struct {
        int flags;
        char options[8];
    } config;
} nested_container_t;

/* More complex nesting */
typedef struct master_type {
    nested_container_t containers[4];
    
    /* Pointer to function returning pointer to array */
    int (*(*get_matrix)[5][5])(void);
    
    /* Union containing struct with callback */
    union {
        struct {
            int_callback handler;
            void* user_data;
        } callback_data;
        struct {
            char* name;
            int value;
        } named_value;
    } variant;
    
    /* Transparent union attribute */
    union {
        int* as_int;
        float* as_float;
        char** as_string;
    } __attribute__((transparent_union)) data_ptr;
} master_type_t;

/* Even more complex: struct containing all type categories */
struct all_types_container {
    /* SCALAR */
    scalar_int count;
    scalar_double precision;
    
    /* STRING */
    const char* title;
    char buffer[1024];
    
    /* STRUCT */
    struct simple_struct simple;
    user_struct_t user;
    
    /* UNION */
    union simple_union data;
    
    /* POINTER */
    void* generic_ptr;
    master_type_t* master;
    
    /* ARRAY */
    int numbers[100];
    struct complex_struct objects[10];
    
    /* CALLBACK */
    void_callback notify;
    
    /* Nested anonymous union with struct */
    union {
        struct {
            int tag;
            float values[4];
        } vector;
        struct {
            char* name;
            int (*compare)(const void*, const void*);
        } comparator;
    } algorithm;
    
    /* Flexible array member */
    int flexible_array[];
};

/* Function pointer type with complex parameter */
typedef void (*event_handler_t)(
    master_type_t* source,
    int event_type,
    const char* message,
    void* user_data,
    int (*filter)(const struct all_types_container*)
);

/* Final comprehensive type that references everything */
typedef struct type_registry {
    undefined_ptr_t undefined_ref;
    scalar_int base_value;
    string_ptr description;
    struct simple_struct* struct_ref;
    user_struct_t user_instance;
    union attributed_union union_instance;
    int_ptr int_pointer;
    int_array_10 int_array;
    int_callback callback;
    struct tree_common* lang_struct_ref;
    nested_container_t nested;
    master_type_t master;
    struct all_types_container* all_types;
    event_handler_t event_handler;
    
    /* Self-referential pointer */
    struct type_registry* next;
} type_registry_t;

#endif /* TEST_TYPES_H */
