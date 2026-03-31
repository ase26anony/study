/* test_gengtype_types.h - Comprehensive type definitions for gengtype coverage */
#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Include gtype.h for GTY macro definition */
#ifdef GENERATOR_FILE
#include "gtype.h"
#else
/* Simplified GTY macro for testing */
#define GTY(x) 
#endif

/* TYPE_SCALAR: Basic scalar typedefs */
typedef int my_int;
typedef unsigned int my_uint;
typedef char my_char;
typedef double my_double;

/* TYPE_STRING: String type definitions */
typedef const char *string_t;
typedef char *mutable_string_t;

/* TYPE_STRUCT: Plain C structs (not GTY-tagged) */
struct plain_struct {
    int field1;
    double field2;
};

/* TYPE_UNION: Plain union */
union plain_union {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(int);
typedef int (*complex_callback)(const char *, void *);

/* TYPE_USER_STRUCT: GTY-tagged structs for garbage collection */
struct GTY(()) user_struct {
    /* TYPE_POINTER: Pointer field */
    struct user_struct *next;
    
    /* TYPE_ARRAY: Array field */
    int values[10];
    
    /* Nested TYPE_STRING */
    const char *name;
    
    /* TYPE_SCALAR field */
    int id;
    
    /* TYPE_CALLBACK field */
    simple_callback cb;
};

/* Another GTY-tagged struct with complex relationships */
struct GTY(()) complex_struct {
    /* Pointer to another GTY-tagged struct */
    struct user_struct *user;
    
    /* Pointer to plain struct */
    struct plain_struct *plain;
    
    /* Array of pointers */
    struct user_struct *children[5];
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Union containing GTY-tagged pointer */
    union {
        struct user_struct *gt_ptr;
        struct plain_struct *plain_ptr;
    } GTY((tag("0"))) choice;
};

/* TYPE_LANG_STRUCT: Language-specific struct with conditional compilation */
#ifdef GENERATOR_FILE
struct GTY(()) lang_specific_struct {
    int lang_field;
    struct user_struct *lang_ptr;
};
#endif

/* Union with GTY tag */
union GTY(()) tagged_union {
    int as_int;
    struct user_struct *as_struct;
    void *as_pointer;
};

/* Struct containing array of callbacks */
struct GTY(()) callback_container {
    simple_callback callbacks[4];
    complex_callback complex_cb;
};

/* Recursive type pattern */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Mixed struct with various type kinds */
struct GTY(()) mixed_types {
    /* Scalar */
    my_int scalar_field;
    
    /* String */
    const char *string_field;
    
    /* Pointer to plain struct */
    struct plain_struct *plain_ptr;
    
    /* Pointer to GTY struct */
    struct user_struct *user_ptr;
    
    /* Array */
    int int_array[20];
    
    /* Array of pointers */
    struct user_struct *ptr_array[8];
    
    /* Callback */
    complex_callback handler;
    
    /* Nested struct (anonymous) */
    struct {
        int nested_field;
        double nested_double;
    } GTY(()) nested;
    
    /* Union field */
    union plain_union data;
};

/* Additional pointer typedefs for TYPE_POINTER coverage */
typedef struct user_struct *user_ptr_t;
typedef struct complex_struct *complex_ptr_t;
typedef void (*void_callback)(void);

/* Struct with all possible GTY options */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
    struct linked_node *child;
};

#endif /* TEST_GENGTYPE_TYPES_H */
