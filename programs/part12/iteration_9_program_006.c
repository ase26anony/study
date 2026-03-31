/* GCC internal type definitions for gengtype coverage testing.
   This file contains diverse type declarations to trigger all
   TYPE_* cases in write_state_type() serialization logic. */

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
typedef unsigned long GTY(()) scalar_ulong;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
extern const char GTY(()) test_string[] = "Hello, gengtype!";
static const char GTY(()) static_string[] = "Static test";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*traversal_func)(void *data, void *user_data);
typedef struct tree_node * GTY((callback)) (*tree_walker)(struct tree_node *);

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct tree_node * GTY(()) tree_ptr;
typedef const struct rtx_def * GTY(()) const_rtx_ptr;
typedef compare_func GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
extern int GTY(()) extern_array[];
typedef struct tree_node * GTY(()) tree_ptr_array[5];
typedef GTY(()) struct {
    int elements[20];
} struct_with_array;

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    float f;
    void *p;
    const char *s;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_FLOAT, TAG_PTR } tag;
    struct {
        int int_val;
    } GTY((desc("%0.tag"))) as_int;
    struct {
        float float_val;
    } GTY((desc("%0.tag"))) as_float;
    struct {
        void *ptr_val;
    } GTY((desc("%0.tag"))) as_ptr;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    int id;
    scalar_int value;
    string_ptr name;
};

struct GTY((chain_next("%h.next"))) linked_node {
    int data;
    struct linked_node * GTY((skip)) next;
    union basic_union value;
};

struct GTY(()) complex_struct {
    int_array numbers;
    int_ptr dynamic_array;
    compare_func comparator;
    union tagged_union variant;
    struct simple_struct embedded;
    struct complex_struct * GTY((skip)) self_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
struct GTY((user)) user_defined {
    int magic;
    void * GTY((skip)) user_data;
    const char *description;
};

typedef struct GTY((user)) {
    int counter;
    struct user_defined * GTY((skip)) ud;
} anonymous_user_struct;

/* TYPE_LANG_STRUCT: GCC language-specific struct types */
struct GTY((length("%h.num_elements"))) lang_struct {
    int num_elements;
    struct tree_node * GTY((length("%0.num_elements"))) elements[1];
};

struct GTY(()) vector_type {
    int GTY((tag("0"))) code;
    union {
        int64_t GTY((tag("1"))) int_val;
        double GTY((tag("2"))) float_val;
    } GTY((desc("%0.code"))) u;
};

/* GCC-specific types that map to internal lang_struct categories */
typedef int GTY(()) __attribute__((vector_size(16))) v4si;
typedef float GTY(()) __attribute__((vector_size(32))) v8sf;

struct GTY(()) gcc_vector_struct {
    v4si int_vector;
    v8sf float_vector;
    struct lang_struct *ls;
};

/* Recursive type structures for deep traversal */
struct GTY(()) container {
    enum { CONT_NONE, CONT_ARRAY, CONT_LIST } type;
    union {
        struct GTY((tag("CONT_ARRAY"))) {
            int size;
            struct container * GTY((length("%0.size"))) items;
        } array;
        struct GTY((tag("CONT_LIST"))) {
            struct container *first;
            struct container *last;
        } list;
    } GTY((desc("%0.type"))) data;
};

/* Nested type with all variations */
struct GTY(()) master_type {
    /* SCALAR */
    scalar_int s_int;
    color_enum color;
    
    /* STRING */
    string_ptr message;
    char GTY(()) fixed_string[50];
    
    /* POINTER */
    int_ptr numbers;
    tree_ptr tree_node;
    callback_ptr handler;
    
    /* ARRAY */
    int_array fixed_array;
    struct tree_node * GTY(()) node_array[5];
    
    /* UNION */
    union basic_union value;
    union tagged_union tagged;
    
    /* STRUCT */
    struct simple_struct simple;
    struct complex_struct complex;
    
    /* USER STRUCT */
    struct user_defined user;
    
    /* LANG STRUCT */
    struct lang_struct lang;
    struct gcc_vector_struct vectors;
    
    /* CALLBACK */
    traversal_func traverse;
    
    /* RECURSIVE */
    struct container * GTY((skip)) recursive;
    
    /* UNDEFINED (pointer to forward declared) */
    struct opaque_struct * GTY((skip)) opaque;
};

/* Global variables to ensure types are instantiated */
extern struct master_type GTY(()) global_master;
extern union basic_union GTY(()) global_union;
extern struct container * GTY(()) global_container;

/* Function types that use these types */
typedef void GTY((callback)) (*master_traverser)(struct master_type *);
typedef struct container * GTY((callback)) (*container_factory)(int);

#endif /* GCC_TEST_COVERAGE_TYPES_H */
