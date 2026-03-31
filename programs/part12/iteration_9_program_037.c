/* test-gtype-coverage.h - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during build */

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
typedef unsigned long GTY(()) scalar_ulong;
typedef _Bool GTY(()) scalar_bool;

/* Enum type (also scalar) */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
extern const char GTY(()) test_string[] = "test string literal";

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef struct GTY(()) array_struct * GTY(()) ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef void * GTY(()) void_ptr;
typedef int * GTY(()) int_ptr;
typedef const char * GTY(()) const_char_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *);
typedef struct GTY(()) tree_node * GTY((callback)) (*alloc_fn)(void);

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) base_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    struct base_struct * GTY(()) next;
};

struct GTY(()) complex_struct {
    scalar_int GTY(()) count;
    string_ptr GTY(()) description;
    fixed_array GTY(()) data;
    compare_fn GTY(()) comparator;
    struct opaque_struct * GTY(()) opaque_ref;
};

/* Struct with nested union */
struct GTY(()) nested_struct {
    int GTY(()) type;
    union {
        int GTY(()) int_val;
        float GTY(()) float_val;
        void_ptr GTY(()) ptr_val;
    } GTY(()) value;
    color_t GTY(()) color;
};

/* TYPE_UNION: Union types */
union GTY(()) data_union {
    int GTY(()) i;
    double GTY(()) d;
    void_ptr GTY(()) p;
    struct base_struct * GTY(()) s;
};

/* Union with struct field */
union GTY(()) complex_union {
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    fixed_array GTY(()) array;
    compare_fn GTY(()) callback;
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
/* Using chain_next for user struct marking */
struct GTY((chain_next("%h.next"))) user_chain_struct {
    int GTY(()) value;
    struct user_chain_struct * GTY(()) next;
    union data_union GTY(()) data;
};

/* Another user struct with nested arrays */
struct GTY(()) user_array_struct {
    int GTY(()) count;
    struct base_struct * GTY(()) items[8];
    ptr_array GTY(()) extra_ptrs;
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific struct types */
/* Using vector types (SIMD) which are often lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    enum tree_code GTY(()) code;
    union tree_node * GTY(()) chain;
    union tree_node * GTY(()) type;
};

struct GTY(()) tree_decl {
    struct tree_common GTY(()) common;
    const char * GTY(()) name;
    struct tree_node * GTY(()) arguments;
};

/* Union tree_node definition */
union GTY((desc("%0.code"))) tree_node {
    struct tree_common GTY(()) common;
    struct tree_decl GTY(()) decl;
    /* Add more tree types as needed */
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int GTY(()) code;
    int GTY(()) mode;
    union {
        int GTY(()) int_val;
        struct rtx_def * GTY(()) rt_ptr;
        compare_fn GTY(()) fn_ptr;
    } GTY(()) u;
};

/* Complex type graph with multiple relationships */
struct GTY(()) master_struct {
    /* Scalar */
    scalar_int GTY(()) id;
    
    /* String */
    string_ptr GTY(()) name;
    
    /* Array */
    fixed_array GTY(()) numbers;
    
    /* Pointer */
    struct master_struct * GTY(()) parent;
    
    /* Struct */
    struct base_struct GTY(()) base;
    
    /* Union */
    union data_union GTY(()) variant;
    
    /* Array of pointers */
    struct master_struct * GTY(()) children[5];
    
    /* Callback */
    traverse_fn GTY(()) traverse;
    
    /* Nested array of structs */
    struct base_struct GTY(()) items[3];
    
    /* Pointer to union */
    union complex_union * GTY(()) complex_ptr;
    
    /* Function pointer returning struct */
    struct base_struct * GTY((callback)) (*allocator)(void);
};

/* Template for generating multiple instances */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct GTY(()) gty_struct_##name { \
        int GTY(()) id; \
        field_type GTY(()) data; \
        struct gty_struct_##name * GTY(()) next; \
    }

/* Instantiate with various types */
DECLARE_GTY_STRUCT(int_ptr, int_ptr);
DECLARE_GTY_STRUCT(string, string_ptr);
DECLARE_GTY_STRUCT(callback, compare_fn);
DECLARE_GTY_STRUCT(array, fixed_array);
DECLARE_GTY_STRUCT(union, union data_union);

/* Self-referential structure for deep graphs */
struct GTY(()) recursive_struct {
    int GTY(()) depth;
    struct recursive_struct * GTY(()) left;
    struct recursive_struct * GTY(()) right;
    union {
        struct recursive_struct * GTY(()) child;
        compare_fn GTY(()) handler;
    } GTY(()) extra;
};

/* Type containing all type kinds */
struct GTY(()) comprehensive_type {
    /* TYPE_SCALAR */
    color_t GTY(()) color;
    
    /* TYPE_STRING */
    const char * GTY(()) message;
    
    /* TYPE_ARRAY */
    int GTY(()) matrix[3][3];
    
    /* TYPE_POINTER */
    void * GTY(()) user_data;
    
    /* TYPE_STRUCT */
    struct base_struct GTY(()) base;
    
    /* TYPE_UNION */
    union data_union GTY(()) data;
    
    /* TYPE_CALLBACK */
    compare_fn GTY(()) compare;
    
    /* Nested anonymous struct */
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    
    /* Array of unions */
    union data_union GTY(()) variants[4];
    
    /* Pointer to function returning pointer */
    struct base_struct * GTY((callback)) (*factory)(int);
    
    /* Chain of user structs */
    struct user_chain_struct * GTY(()) chain;
    
    /* Lang struct pointer */
    union tree_node * GTY(()) tree;
    
    /* Vector type */
    v4si GTY(()) vector;
};

/* Global variables to ensure processing */
extern struct comprehensive_type GTY(()) global_comprehensive;
extern union tree_node * GTY(()) global_tree;
extern struct rtx_def * GTY(()) global_rtx;
extern struct user_chain_struct * GTY(()) global_chain;

#endif /* TEST_GTYPE_COVERAGE_H */
