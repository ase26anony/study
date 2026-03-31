/* test-gtype-coverage.c - Comprehensive GTY type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types with GTY markers */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_enum;

typedef _Bool GTY(()) gty_bool;
typedef int GTY(()) gty_int;
typedef long GTY(()) gty_long;
typedef unsigned GTY(()) gty_unsigned;

/* TYPE_STRING: String types */
typedef const char *GTY(()) gty_string;
static const char *GTY(()) global_string = "test string";
static char GTY(()) string_array[] = "array string";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*gty_callback)(void *);
typedef void GTY((callback)) (*void_callback)(int, const char *);

/* TYPE_POINTER: Various pointer types */
typedef struct simple_struct *GTY(()) simple_struct_ptr;
typedef void *GTY(()) gty_void_ptr;
typedef int *GTY(()) gty_int_ptr;
typedef gty_callback *GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef struct simple_struct *GTY(()) ptr_array[5];
typedef GTY(()) int multi_dim_array[3][4];

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    float f;
    double d;
    void *p;
};

union GTY(()) nested_union {
    struct simple_struct *sptr;
    union basic_union u;
    fixed_array arr;
};

/* TYPE_STRUCT: Basic struct types */
struct GTY(()) simple_struct {
    int id;
    char *GTY(()) name;
    struct simple_struct *GTY(()) next;
    union basic_union GTY(()) data;
};

/* More complex struct with various field types */
struct GTY(()) complex_struct {
    /* Scalar fields */
    gty_int count;
    color_enum color;
    
    /* String field */
    gty_string description;
    
    /* Pointer fields */
    simple_struct_ptr first;
    gty_void_ptr user_data;
    
    /* Array field */
    fixed_array values;
    
    /* Union field */
    union nested_union GTY(()) choice;
    
    /* Callback field */
    gty_callback callback;
    
    /* Nested struct pointer */
    struct complex_struct *GTY(()) sibling;
    
    /* Incomplete array */
    int GTY(()) flexible_array[];
};

/* Chainable struct with special GTY options */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int value;
    struct linked_node *GTY(()) next;
    struct linked_node *GTY(()) prev;
    gty_string GTY(()) label;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
    int tag;
    union {
        int ival;
        float fval;
        char *GTY(()) sval;
    } GTY(()) data;
    void (*GTY((skip)) process)(struct user_struct *);
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific structures */

/* Vector type (GCC extension) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure (mimicking GCC internals) */
struct GTY(()) tree_common {
    int code;
    union tree_node *GTY(()) chain;
    union tree_node *GTY(()) type;
};

struct GTY(()) tree_int_cst {
    struct tree_common common;
    long int value;
};

union GTY(()) tree_node {
    struct tree_common common;
    struct tree_int_cst int_cst;
};

/* RTL-like structure (another GCC internal) */
struct GTY(()) rtx_def {
    int code;
    int mode;
    union {
        long int GTY(()) intval;
        struct rtx_def *GTY(()) rtx;
        const char *GTY(()) str;
    } GTY(()) u;
    struct rtx_def *GTY(()) next;
};

/* Array of unions */
union GTY(()) variant_array[8];

/* Struct containing array of pointers to callbacks */
struct GTY(()) callback_container {
    int count;
    gty_callback GTY(()) handlers[4];
    void_callback GTY(()) notifiers[2];
};

/* Nested type definitions to create complex type graph */
typedef struct GTY(()) outer_struct {
    int id;
    struct GTY(()) inner_struct {
        int value;
        struct outer_struct *GTY(()) parent;
        union GTY(()) {
            int num;
            char *GTY(()) str;
        } data;
    } *GTY(()) inner;
    struct callback_container GTY(()) callbacks;
} outer_struct_t;

/* Global variables with various types to ensure processing */
struct simple_struct GTY(()) global_struct = {0};
union basic_union GTY(()) global_union;
outer_struct_t *GTY(()) global_outer = NULL;
struct callback_container GTY(()) global_callbacks;

/* Function pointer table */
static gty_callback GTY(()) callback_table[] = {
    NULL,
    NULL
};

/* Mixed array */
static GTY(()) void *mixed_array[] = {
    &global_struct,
    &global_union,
    global_string,
    NULL
};

/* Recursive type definition */
typedef struct GTY(()) recursive_node {
    int data;
    struct recursive_node *GTY(()) left;
    struct recursive_node *GTY(()) right;
    gty_string GTY(()) label;
} recursive_node_t;

/* Union containing struct with callback */
union GTY(()) union_with_callback {
    struct GTY(()) {
        int type;
        gty_callback handler;
    } s;
    long int fallback;
};

/* Complete the forward declarations */
struct GTY(()) opaque_struct {
    int revealed;
    struct opaque_struct *GTY(()) next;
};

union GTY(()) opaque_union {
    int i;
    struct opaque_struct *GTY(()) s;
};

/* Template for language-specific type (C++ like) */
#ifdef __cplusplus
struct GTY(()) template_struct {
    void *GTY(()) data;
    int (*GTY((callback)) compare)(const void *, const void *);
};
#endif

/* Mark the end of types */
static int GTY(()) coverage_check = 1;
