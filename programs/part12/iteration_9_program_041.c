/* test-gtype-coverage.h - Comprehensive type definitions for gengtype coverage */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
extern const char GTY(()) *string_ptr;
static const char GTY(()) fixed_string[] = "Hello, gengtype!";
typedef char GTY(()) string_array[32];

/* TYPE_POINTER: Various pointer types */
typedef void GTY(()) *void_ptr;
typedef int GTY(()) *int_ptr;
typedef struct my_struct GTY(()) *struct_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*traverse_func)(void *);
typedef struct my_struct GTY((callback)) *(*allocator_func)(size_t);

/* TYPE_ARRAY: Array types */
extern int GTY(()) incomplete_array[];
typedef int GTY(()) fixed_size_array[10];
typedef struct my_struct GTY(()) *ptr_array[5];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
    int GTY(()) i;
    float GTY(()) f;
    void GTY(()) *p;
    struct my_struct GTY(()) *s;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
    /* Scalar fields */
    int GTY(()) id;
    color_enum GTY(()) color;
    
    /* String field */
    const char GTY(()) *name;
    
    /* Pointer fields */
    struct my_struct GTY(()) *next;
    void_ptr GTY(()) data;
    
    /* Array field */
    int GTY(()) values[5];
    
    /* Union field */
    union my_union GTY(()) u;
    
    /* Callback field */
    compare_func GTY(()) compare;
};

/* More complex struct with nested structures */
struct GTY(()) complex_struct {
    struct my_struct GTY(()) *first;
    struct my_struct GTY(()) **GTY((length("%h.count"))) items;
    int GTY(()) count;
    
    /* Array of unions */
    union my_union GTY(()) union_array[3];
    
    /* Function pointer array */
    traverse_func GTY(()) handlers[2];
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
    int GTY(()) tag;
    union {
        int GTY(()) i;
        float GTY(()) f;
    } GTY((desc("%0.tag"))) value;
};

/* Recursive structure for deep traversal */
struct GTY((chain_next("%h.next"))) recursive_struct {
    int GTY(()) depth;
    struct recursive_struct GTY(()) *next;
    struct recursive_struct GTY(()) *children[3];
};

/* TYPE_LANG_STRUCT: GCC-specific types */
/* Vector types using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    int GTY(()) code;
    union tree_node GTY(()) *chain;
    union tree_node GTY(()) *type;
};

union GTY(()) tree_node {
    struct tree_common GTY(()) common;
    /* In real GCC, there would be many more variants here */
    struct {
        struct tree_common GTY(()) common;
        long GTY(()) int_value;
    } GTY(()) integer;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int GTY(()) code;
    union {
        long GTY(()) int_val;
        struct rtx_def GTY(()) *rtx_ptr;
    } GTY((desc("%0.code"))) u;
};

/* More complex type combinations */

/* Struct containing array of pointers to callbacks */
struct GTY(()) callback_container {
    compare_func GTY(()) callbacks[4];
    int GTY(()) active_count;
};

/* Union with struct members */
union GTY(()) nested_union {
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    struct {
        const char GTY(()) *name;
        int GTY(()) id;
    } GTY(()) info;
};

/* Self-referential types */
typedef struct self_ref GTY(()) self_ref_t;
struct GTY(()) self_ref {
    int GTY(()) data;
    self_ref_t GTY(()) *next;
    self_ref_t GTY(()) *prev;
};

/* Container with multiple type kinds */
struct GTY(()) type_container {
    /* Scalar */
    scalar_int GTY(()) num;
    
    /* String */
    const char GTY(()) *message;
    
    /* Pointer */
    void_ptr GTY(()) generic_ptr;
    
    /* Array */
    fixed_size_array GTY(()) numbers;
    
    /* Struct */
    struct my_struct GTY(()) embedded_struct;
    
    /* Union */
    union my_union GTY(()) choice;
    
    /* Callback */
    traverse_func GTY(()) handler;
    
    /* Nested container */
    struct type_container GTY(()) *nested;
};

/* Global variables to ensure types are used */
extern struct my_struct GTY(()) *global_struct_list;
extern union my_union GTY(()) global_union_var;
extern compare_func GTY(()) global_comparator;
extern int GTY(()) global_array[20];

/* Inline function using the types */
static inline void GTY(()) process_struct(struct my_struct GTY(()) *s) {
    if (s && s->compare) {
        s->compare(s->values, s->u.p);
    }
}

#endif /* TEST_GTYPE_COVERAGE_H */
