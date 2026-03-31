/* test-gtype-coverage.h - Comprehensive type definitions for gengtype coverage */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_struct;
union opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String types */
const char *global_string = "Hello, gengtype!";
char string_array[] = "Test string array";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_struct *opaque_ptr;
typedef const char *const_string_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*simple_callback)(void);
typedef struct my_struct *(*factory_fn)(int);

/* TYPE_ARRAY: Array types */
extern int incomplete_array[];
int fixed_array[10];
const char *string_ptr_array[5];

/* TYPE_STRUCT: Regular struct types */
struct simple_struct {
    int id;
    char name[32];
    struct simple_struct *next;
};

struct nested_struct {
    int value;
    struct {
        int x;
        int y;
    } point;
    struct nested_struct *child;
};

/* TYPE_UNION: Union types */
union data_union {
    int i;
    float f;
    double d;
    void *p;
    char str[16];
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

/* GTY-annotated types for garbage collection */

/* TYPE_STRUCT with GTY annotation */
struct GTY((chain_next("%h.next"))) gty_struct {
    int GTY((skip)) id;  /* skip field - not traced */
    char *GTY((tag("0"))) name;
    struct gty_struct *next;
    union data_union data;
};

/* TYPE_UNION with GTY annotation */
union GTY((desc("%0.type"))) gty_union {
    int type;
    struct GTY((tag("1"))) {
        int x, y;
    } point;
    struct GTY((tag("2"))) {
        float radius;
        char *name;
    } circle;
};

/* TYPE_ARRAY with GTY annotation */
typedef struct gty_struct * GTY((length("%h.count"))) gty_struct_ptr_array;
struct GTY(()) array_container {
    int count;
    gty_struct_ptr_array items;
};

/* TYPE_POINTER with special GTY options */
typedef union gty_union * GTY((atomic)) gty_union_ptr;

/* TYPE_CALLBACK with GTY annotation */
typedef int GTY((callback)) (*gty_callback_fn)(struct gty_struct *);

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* These typically require GCC-specific extensions */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union GTY((desc("%0.code"))) tree_node {
    struct tree_common common;
    struct GTY((tag("1"))) {
        struct tree_common common;
        long int_cst;
    } integer_cst;
    struct GTY((tag("2"))) {
        struct tree_common common;
        const char *pointer;
    } string_cst;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_container {
    /* TYPE_STRUCT */
    struct gty_struct *struct_ptr;
    
    /* TYPE_UNION */
    union gty_union data_union;
    
    /* TYPE_ARRAY */
    struct gty_struct *struct_array[5];
    
    /* TYPE_POINTER */
    void **pointer_array;
    
    /* TYPE_CALLBACK */
    gty_callback_fn callback;
    
    /* TYPE_SCALAR */
    color_enum color;
    
    /* TYPE_STRING */
    const char *description;
    
    /* Recursive reference */
    struct complex_container *next;
};

/* External declarations to create TYPE_UNDEFINED references */
extern struct undefined_external *external_ptr;
extern union undefined_union *external_union_ptr;

/* Function prototypes using various types */
struct gty_struct *create_struct(int id, const char *name);
void process_union(union gty_union *u);
int compare_structs(const struct gty_struct *a, const struct gty_struct *b);

/* Template for generating multiple instances */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        int id; \
        field_type data; \
        struct name##_struct *next; \
    }

/* Generate multiple struct types */
DECLARE_STRUCT_TYPE(int, int);
DECLARE_STRUCT_TYPE(ptr, void*);
DECLARE_STRUCT_TYPE(callback, gty_callback_fn);

/* Array of different callback types */
typedef void (*void_callback)(void);
typedef int (*int_callback)(int);
typedef char *(*string_callback)(const char*);

union GTY(()) callback_union {
    void_callback vcb;
    int_callback icb;
    string_callback scb;
};

/* Multi-dimensional array case */
struct GTY(()) matrix_container {
    int matrix[3][3];
    struct gty_struct *ptr_matrix[2][2];
};

/* Self-referential type with multiple paths */
struct GTY(()) graph_node {
    int id;
    struct graph_node **neighbors;  /* Array of pointers */
    int neighbor_count;
};

/* Type with conditional fields */
struct GTY((desc("%0.type"))) conditional_struct {
    int type;
    union {
        int int_value;
        float float_value;
        struct gty_struct *struct_ptr;
        const char *string_value;
    } GTY((tag("0.type"))) data;
};

#endif /* TEST_GTYPE_COVERAGE_H */
