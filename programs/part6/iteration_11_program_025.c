/* gcc-test-types.h - Comprehensive type definitions for gengtype coverage testing */
#ifndef GCC_TEST_TYPES_H
#define GCC_TEST_TYPES_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_STRUCT: Basic struct types */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    enum color color;
};

typedef struct GTY(()) point {
    double x;
    double y;
} point_t;

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined {
    int user_id;
    void * GTY((skip)) user_data;  /* Skip for GC */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;
    struct basic_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct struct_array_5[5];

/* TYPE_STRING: String type */
struct GTY(()) gcc_string {
    int length;
    char * GTY((length("((struct gcc_string *)&_n)->length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *context, int value);
typedef void (*simple_callback_t)(void);

/* TYPE_LANG_STRUCT: Language-specific struct (simulated) */
struct GTY(()) lang_specific {
    int lang_tag;
    union data_union lang_data;
    callback_func_t lang_callback;
};

/* Complex nested structures to ensure deep traversal */

/* Linked list node - creates pointer chains */
struct GTY(()) list_node {
    int value;
    struct list_node * GTY((tag("0"))) next;
    struct list_node * GTY((tag("1"))) prev;
};

/* Tree node with multiple pointer types */
struct GTY(()) tree_node {
    int key;
    struct tree_node *left;
    struct tree_node *right;
    struct gcc_string *label;
    callback_func_t visitor;
};

/* Container with arrays of different types */
struct GTY(()) type_container {
    /* Scalar members */
    int count;
    double total;
    
    /* Struct member */
    struct basic_struct header;
    
    /* Union member */
    union data_union current_data;
    
    /* Pointer members */
    struct list_node *first;
    struct tree_node *root;
    struct gcc_string *description;
    
    /* Array members */
    int scores[5];
    struct basic_struct items[3];
    
    /* Function pointer */
    callback_func_t processor;
    
    /* Nested struct */
    struct {
        int nested_id;
        char nested_name[20];
    } GTY(()) metadata;
};

/* Mixed annotated and non-annotated types for TYPE_UNDEFINED testing */

/* Non-annotated struct (may be TYPE_UNDEFINED if referenced) */
struct non_gty_struct {
    int internal_data;
    float internal_float;
};

/* Annotated struct referencing non-annotated type */
struct GTY(()) annotated_with_undefined {
    int id;
    struct non_gty_struct * GTY((skip)) undefined_ref;  /* Skip GC for non-GTY type */
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* Basic types */
    scalar_int_t base_int;
    scalar_double_t base_double;
    
    /* Struct types */
    struct basic_struct *basic;
    point_t *point;
    struct user_defined *user;
    
    /* Union type */
    union data_union *data;
    
    /* Pointer types */
    int_ptr_t numbers;
    generic_ptr_t generic;
    
    /* Array types */
    int_array_10 fixed_array;
    struct_array_5 structs;
    
    /* String type */
    struct gcc_string *text;
    
    /* Callback type */
    callback_func_t handler;
    
    /* Language struct */
    struct lang_specific *lang;
    
    /* Complex structures */
    struct list_node *list_head;
    struct tree_node *tree_root;
    struct type_container *container;
    
    /* Reference to undefined type */
    struct annotated_with_undefined *with_undefined;
    
    /* Array of pointers */
    struct basic_struct * GTY((length("10"))) ptr_array[10];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Global root variable */
extern struct root_container GTY(()) global_root;

#endif /* GCC_TEST_TYPES_H */
