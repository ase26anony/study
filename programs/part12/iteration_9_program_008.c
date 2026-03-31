/* Test file to trigger all TYPE_* cases in gengtype-state.cc write_state_type()
   This file should be placed in the gcc/ directory and included in the build.
   All types are annotated with GTY markers to ensure gengtype processes them. */

#ifndef GCC_TEST_COVERAGE_TYPES_H
#define GCC_TEST_COVERAGE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef long GTY(()) scalar_long;
typedef unsigned long GTY(()) scalar_ulong;

/* Scalar enum type */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
typedef char GTY(()) string_array[32];

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*void_callback)(void);
typedef struct my_struct * GTY((callback)) (*struct_factory)(int);

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct my_struct * GTY(()) struct_ptr;
typedef union my_union * GTY(()) union_ptr;
typedef int (* GTY(()) func_ptr)(void);
typedef compare_fn GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) fixed_array[10];
typedef struct my_struct * GTY(()) ptr_array[5];
typedef char GTY(()) string_literal_array[];
extern int GTY(()) incomplete_array[];

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
    /* Scalar fields */
    int GTY(()) id;
    scalar_int GTY(()) value;
    color GTY(()) col;
    
    /* Pointer fields */
    int_ptr GTY(()) numbers;
    struct_ptr GTY(()) next;
    void_ptr GTY(()) data;
    
    /* Array field */
    fixed_array GTY(()) scores;
    
    /* String field */
    string_ptr GTY(()) name;
    
    /* Callback field */
    compare_fn GTY(()) comparator;
    
    /* Nested struct pointer */
    struct nested_struct * GTY(()) nested;
    
    /* Union field */
    union my_union * GTY(()) variant;
};

/* Another struct with chain_next for GC */
struct GTY((chain_next("%h.next"))) linked_node {
    int GTY(()) data;
    struct linked_node * GTY(()) next;
    struct linked_node * GTY(()) prev;
};

/* Struct containing an array of pointers */
struct GTY(()) container {
    int GTY(()) count;
    struct my_struct * GTY(()) items[8];
    ptr_array GTY(()) more_items;
};

/* TYPE_UNION: Union types */
union GTY(()) my_union {
    int GTY(()) i;
    float GTY(()) f;
    double GTY(()) d;
    void_ptr GTY(()) p;
    struct_ptr GTY(()) s;
    string_ptr GTY(()) str;
};

/* Union with tag for discriminant */
union GTY(()) tagged_union {
    struct GTY((desc("%0.type"))) {
        int GTY(()) type;
    } header;
    
    struct GTY((tag("0"))) {
        int GTY(()) int_value;
    } int_case;
    
    struct GTY((tag("1"))) {
        string_ptr GTY(()) string_value;
    } string_case;
    
    struct GTY((tag("2"))) {
        struct_ptr GTY(()) struct_value;
    } struct_case;
};

/* TYPE_USER_STRUCT: Struct with user-defined properties */
struct GTY((user)) user_defined {
    int GTY(()) uid;
    char GTY(()) *username;
    struct user_defined * GTY(()) friends[10];
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific struct types */

/* Vector type using GCC extension (often treated as lang_struct) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC's internal tree_node */
struct GTY(()) tree_common {
    int GTY(()) code;
    union tree_node * GTY(()) chain;
    union tree_node * GTY(())) type;
    int GTY(())) side_effects_flag;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int GTY(()) code;
    int GTY(()) mode;
    union GTY(()) {
        long GTY(()) intval;
        double GTY(()) dval;
        char * GTY(()) str;
        struct GTY(()) {
            struct rtx_def * GTY(()) first;
            struct rtx_def * GTY(()) second;
        } pair;
    } u;
};

/* Forward reference to union tree_node */
union GTY(()) tree_node;

/* Complete the tree_node union definition */
union GTY(()) tree_node {
    struct tree_common GTY(()) common;
    struct GTY((tag("0"))) {
        struct tree_common GTY(()) common;
        long GTY(()) intval;
    } integer;
    struct GTY((tag("1"))) {
        struct tree_common GTY(()) common;
        double GTY(()) realval;
    } real;
    struct GTY((tag("2"))) {
        struct tree_common GTY(()) common;
        char * GTY(()) pointer;
        long GTY(()) length;
    } string;
};

/* Complex nested type combining multiple type kinds */
struct GTY(()) complex_type {
    /* Direct fields of various types */
    int GTY(()) scalar_field;
    string_ptr GTY(()) string_field;
    v4si GTY(()) vector_field;
    
    /* Nested struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } point;
    
    /* Union field */
    union my_union GTY(()) data;
    
    /* Array of different pointer types */
    void * GTY(()) pointers[4];
    
    /* Function pointer with complex signature */
    struct my_struct * (* GTY(()) factory)(
        int,
        const char *,
        compare_fn
    );
    
    /* Pointer to lang_struct */
    union tree_node * GTY(()) tree;
    
    /* Pointer to user_struct */
    struct user_defined * GTY(()) user;
    
    /* Self-referential pointer */
    struct complex_type * GTY(()) self;
    
    /* Array of callback pointers */
    compare_fn GTY(()) comparators[3];
};

/* Global variables to ensure types are instantiated */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern struct complex_type * GTY(()) global_complex;
extern string_ptr GTY(()) global_strings[];
extern compare_fn GTY(()) global_callbacks[2];

/* Inline function using the types (not processed by gengtype but ensures compilation) */
static inline void
test_types_function(void)
{
    struct my_struct local_struct = {0};
    union my_union local_union;
    struct complex_type *local_complex = NULL;
    
    local_struct.id = 1;
    local_struct.name = "test";
    local_union.i = 42;
    
    if (local_complex)
        local_complex->scalar_field = local_struct.id;
}

/* Template for generating multiple instances (useful for array testing) */
#define DECLARE_STRUCT_ARRAY(name, size) \
    struct my_struct GTY(()) name[size]

/* Declare some template instances */
DECLARE_STRUCT_ARRAY(struct_array_1, 5);
DECLARE_STRUCT_ARRAY(struct_array_2, 10);

#endif /* GCC_TEST_COVERAGE_TYPES_H */
