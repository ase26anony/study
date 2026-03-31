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
typedef long long scalar_ll;
typedef unsigned int scalar_uint;

/* TYPE_STRING: String types and literals */
typedef const char* string_ptr;
#define STRING_LITERAL "test_string_literal"

/* TYPE_STRUCT: Complete struct definitions */
struct simple_struct {
    scalar_int id;
    scalar_float value;
    char name[32];
};

struct complex_struct {
    struct simple_struct base;
    struct complex_struct *next;
    void *data;
    int array[10];
};

/* TYPE_USER_STRUCT: Typedef'd struct types */
typedef struct {
    scalar_int x;
    scalar_double y;
    char label[64];
} user_struct_t;

typedef struct nested_user_struct {
    user_struct_t user;
    struct nested_user_struct *parent;
    int depth;
} nested_user_t;

/* TYPE_UNION: Union types */
union simple_union {
    scalar_int as_int;
    scalar_float as_float;
    scalar_char as_char;
    void *as_ptr;
};

union complex_union {
    struct simple_struct as_struct;
    user_struct_t as_user;
    union simple_union as_simple;
    long long as_ll;
};

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *int_ptr;
typedef struct simple_struct *struct_ptr;
typedef union simple_union *union_ptr;
typedef user_struct_t *user_ptr;
typedef void (*func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Arrays of different types */
typedef int int_array_10[10];
typedef struct simple_struct struct_array_5[5];
typedef user_struct_t user_array_3[3];
typedef int_ptr pointer_array_8[8];
typedef char string_array[4][32];

/* TYPE_CALLBACK: Function pointer types with various signatures */
typedef int (*int_callback)(void);
typedef void (*void_callback)(int, float);
typedef struct simple_struct* (*struct_callback)(user_struct_t*, int);
typedef union complex_union (*union_callback)(int_array_10);
typedef void (*complex_callback)(int (*)(float), void*);

/* TYPE_LANG_STRUCT: GCC internal-like structures */
/* These names might be recognized by gengtype as language-specific */
struct tree_common;
struct tree_type;
struct tree_decl;
struct tree_node;

/* Complex nested type definitions with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    scalar_ll data[4];
    char padding[32];
};

union __attribute__((packed)) packed_union {
    struct aligned_struct as_aligned;
    user_struct_t as_user;
    unsigned char bytes[64];
};

struct __attribute__((transparent_union)) transparent_union_wrapper {
    union {
        int *int_ptr;
        void *void_ptr;
        const char *str_ptr;
    } u;
};

/* Deeply nested type hierarchy */
typedef struct container {
    /* Nested struct containing array of pointers to unions */
    struct {
        union complex_union *union_ptrs[5];
        int count;
    } union_container;
    
    /* Function pointer returning pointer to struct with callback */
    struct complex_struct* (*get_complex)(int id, 
        void (*callback)(struct complex_struct*));
    
    /* Multi-dimensional array */
    int matrix[3][3][3];
    
    /* Mixed member types */
    scalar_int scalar_member;
    string_ptr string_member;
    struct simple_struct struct_member;
    user_struct_t user_member;
    union simple_union union_member;
    int_ptr pointer_member;
    int_array_10 array_member;
    int_callback callback_member;
    
    /* Pointer to language-like struct */
    struct tree_node *lang_struct_ptr;
} container_t;

/* More complex typedef chains */
typedef container_t* container_ptr;
typedef container_ptr (*container_factory)(int, const char*);
typedef void (*container_processor)(container_t**, int);

/* Template for generating multiple similar types */
#define DECLARE_VECTOR_TYPE(T, N) \
    typedef struct vector_##T##_##N { \
        T data[N]; \
        int size; \
    } vector_##T##_##N##_t

DECLARE_VECTOR_TYPE(int, 10);
DECLARE_VECTOR_TYPE(float, 8);
DECLARE_VECTOR_TYPE(struct simple_struct*, 5);

/* Opaque handle type */
typedef struct opaque_handle* handle_t;

/* Self-referential structures */
struct recursive_node {
    int value;
    struct recursive_node *left;
    struct recursive_node *right;
    void (*visit)(struct recursive_node*);
};

/* Const-qualified types */
typedef const int const_int;
typedef const struct simple_struct const_struct;
typedef const user_struct_t* const_user_ptr;

/* Volatile-qualified types */
typedef volatile int volatile_int;
typedef volatile struct complex_struct* volatile_struct_ptr;

/* Function type typedefs */
typedef int binary_func(int, int);
typedef void cleanup_func(void*);

/* Anonymous struct/union in typedef */
typedef struct {
    union {
        int x;
        float y;
    } coord;
    int type;
} anonymous_t;

#endif /* TEST_TYPES_H */
