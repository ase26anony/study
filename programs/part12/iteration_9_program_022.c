/* test-gtype-coverage.h - Comprehensive GTY type coverage test for gengtype */
/* This file should be placed in gcc/ directory and included in gtype-desc.c */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;
typedef long long GTY(()) scalar_ll;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
typedef char GTY(()) string_array[32];

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*void_callback)(void);
typedef struct test_struct * GTY((callback)) (*struct_factory)(int);

/* TYPE_POINTER: Various pointer types */
typedef scalar_int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct test_struct * GTY(()) struct_ptr;
typedef union test_union * GTY(()) union_ptr;
typedef compare_fn * GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct test_struct * GTY(()) struct_ptr_array[5];
typedef GTY(()) int incomplete_array[];
typedef GTY(()) char string_literal_array[];

/* TYPE_UNION: Union types */
union GTY(()) test_union {
    int i;
    float f;
    void *p;
    struct test_struct *s;
    int_array arr;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_PTR, TAG_ARRAY } tag;
    struct {
        int value;
    } GTY((tag("TAG_INT"))) as_int;
    struct {
        void *ptr;
    } GTY((tag("TAG_PTR"))) as_ptr;
    struct {
        int_array arr;
    } GTY((tag("TAG_ARRAY"))) as_array;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) test_struct {
    /* Scalar fields */
    scalar_int id;
    color_enum color;
    
    /* String field */
    string_ptr name;
    
    /* Pointer fields */
    struct_ptr next;
    void_ptr data;
    
    /* Array field */
    int_array values;
    
    /* Union field */
    union test_union variant;
    
    /* Callback field */
    compare_fn comparator;
};

/* Chainable struct with special GTY options */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chainable_struct {
    int value;
    struct chainable_struct * GTY((skip)) next;
    struct chainable_struct *prev;
};

/* Nested struct with arrays of pointers */
struct GTY(()) complex_container {
    /* Array of struct pointers */
    struct_ptr_array items;
    
    /* Pointer to array */
    int_array *array_ptr;
    
    /* Multi-dimensional array */
    int GTY(()) matrix[3][3];
    
    /* Union containing struct */
    union {
        struct test_struct s;
        struct chainable_struct c;
    } GTY(()) container;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal-like types */
/* Using GCC vector extensions to trigger lang_struct handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));
typedef float GTY(()) v4sf __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    enum tree_code {
        ERROR_MARK,
        IDENTIFIER_NODE,
        TREE_LIST,
        VOID_TYPE,
        INTEGER_TYPE
    } code;
    union tree_node *chain;
};

union GTY((desc("%0.code"))) tree_node {
    struct tree_common common;
    struct GTY((tag("INTEGER_TYPE"))) tree_type {
        struct tree_common common;
        int precision;
        union tree_node *pointer_to;
    } type;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int code;
    union GTY((desc("%0.code"))) rtunion_def {
        int GTY((tag("0"))) rtwint;
        struct GTY((tag("1"))) rtx_def *rtx;
        struct GTY((tag("2"))) chainable_struct *rtstruct;
    } u;
};

/* TYPE_LANG_STRUCT with variable-length array */
struct GTY(()) variable_size_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* Recursive type definitions to ensure deep traversal */
typedef struct GTY(()) recursive_struct recursive_struct_t;
struct GTY(()) recursive_struct {
    int value;
    recursive_struct_t *left;
    recursive_struct_t *right;
    union {
        recursive_struct_t *alias;
        void *generic;
    } GTY(()) link;
};

/* Template for generating multiple instantiations */
#define DECLARE_STRUCT_TYPE(T) \
    struct GTY(()) struct_##T { \
        T value; \
        struct struct_##T *next; \
    }

DECLARE_STRUCT_TYPE(int);
DECLARE_STRUCT_TYPE(float);
DECLARE_STRUCT_TYPE(struct test_struct*);

/* Global variables with GTY annotations to ensure processing */
extern struct test_struct GTY(()) *global_struct_ptr;
extern union test_union GTY(()) global_union;
extern int_array GTY(()) global_array;
extern compare_fn GTY(()) global_callback;
extern const char GTY(()) *global_string;

/* Inline function using types to ensure they're referenced */
static inline void GTY(()) use_all_types(void) {
    struct test_struct local_struct = {0};
    union test_union local_union;
    int_array local_array = {0};
    compare_fn local_callback = 0;
    
    /* Reference all types to avoid unused warnings */
    (void)local_struct;
    (void)local_union;
    (void)local_array;
    (void)local_callback;
}

#endif /* TEST_GTYPE_COVERAGE_H */
