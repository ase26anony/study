/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;

enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void * GTY((skip)) opaque_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;
    struct basic_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef struct basic_struct *struct_ptr_array_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char * GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *context, int value);
typedef void (*cleanup_func_t)(struct basic_struct *obj);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    union data_union lang_data;
    callback_func_t lang_callback;
};

/* Complex nested structures to ensure deep traversal */

/* Linked list node (TYPE_STRUCT with TYPE_POINTER member) */
struct GTY(()) list_node {
    int data;
    struct list_node * GTY((skip)) next;  /* Skip for circular reference */
    struct list_node *prev;
};

/* Tree node with multiple pointer types */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct gcc_string *label;
    callback_func_t visitor;
};

/* Container with arrays of different types */
struct GTY(()) type_container {
    /* TYPE_ARRAY members */
    int scalar_array[8];
    struct basic_struct *ptr_array[4];
    union data_union union_array[3];
    
    /* TYPE_POINTER members */
    struct undefined_struct *undefined_ptr;  /* References TYPE_UNDEFINED */
    struct user_struct *user_ptr;           /* References TYPE_USER_STRUCT */
    struct lang_struct *lang_ptr;           /* References TYPE_LANG_STRUCT */
    
    /* TYPE_CALLBACK member */
    cleanup_func_t cleanup;
    
    /* TYPE_STRING member */
    struct gcc_string description;
    
    /* TYPE_UNION member */
    union data_union current_data;
    
    /* TYPE_SCALAR members */
    enum color color;
    scalar_double_t weight;
};

/* Root structure containing all types */
struct GTY(()) root_struct {
    /* Direct instances */
    struct basic_struct basic;
    struct user_struct user;
    struct lang_struct lang;
    struct gcc_string title;
    
    /* Pointers to instances */
    struct type_container *container;
    struct tree_node *tree_root;
    struct list_node *list_head;
    
    /* Arrays */
    struct basic_struct struct_array[3];
    union data_union union_array[2];
    
    /* Callback */
    callback_func_t notify;
    
    /* Reference to undefined type */
    struct undefined_struct *undefined_ref;
};

/* Global root variable */
extern struct root_struct GTY((root)) global_root;

#endif /* GTY_TEST_H */
