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

/* TYPE_UNDEFINED: Non-GTY annotated struct that will be referenced */
struct undefined_helper {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* GTY((skip)) opaque_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char* string_val;
    struct basic_struct* struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* struct_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct* struct_ptr_array[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char* GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func)(void* context, int value);
typedef void (*cleanup_func)(struct basic_struct* obj);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    union data_union lang_data;
    callback_func lang_callback;
};

/* Complex nested structures to ensure deep traversal */

/* Linked list node (TYPE_STRUCT with TYPE_POINTER member) */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((skip)) next;  /* Skip to avoid infinite recursion in test */
    struct list_node* GTY((skip)) prev;
};

/* Tree node structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
    struct gcc_string* label;
};

/* Container with arrays of different types */
struct GTY(()) type_container {
    /* TYPE_ARRAY members */
    int scalar_array[20];
    struct basic_struct* ptr_array[8];
    union data_union union_array[4];
    
    /* TYPE_POINTER members */
    struct undefined_helper* undefined_ptr;  /* Will trigger TYPE_UNDEFINED */
    struct gcc_string* string_ptr;
    
    /* TYPE_CALLBACK member */
    callback_func callback;
    
    /* TYPE_UNION member */
    union data_union current_data;
    
    /* TYPE_SCALAR members */
    enum color color;
    long long big_scalar;
    
    /* Nested TYPE_STRUCT */
    struct {
        int nested_id;
        float nested_value;
    } GTY((tag("0"))) nested;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Direct struct instances */
    struct basic_struct basic;
    struct user_struct user;
    struct lang_struct lang;
    
    /* Pointers to various types */
    struct type_container* container;
    struct list_node* list_head;
    struct tree_node* tree_root;
    
    /* Arrays */
    struct gcc_string* string_array[3];
    callback_func callback_array[2];
    
    /* Union */
    union data_union root_union;
    
    /* String */
    struct gcc_string root_string;
    
    /* Scalar */
    unsigned long root_scalar;
    
    /* Pointer to undefined type */
    struct undefined_helper* helper;
};

/* Global variable declarations for root tracking */
extern struct root_container GTY((root)) global_root;

/* Function pointer table */
struct GTY(()) callback_table {
    const char* name;
    callback_func func;
    cleanup_func cleanup;
};

#endif /* GTY_TEST_H */
