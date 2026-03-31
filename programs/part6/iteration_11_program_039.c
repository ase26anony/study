/* Test header for gengtype-state.cc coverage testing */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* TYPE_ENUM (handled as scalar) */
enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Forward declaration without GTY marker */
struct undefined_struct;

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) basic_ptr;  /* Pointer to another struct */
    int GTY((tag)) tag_field;
    union internal_union *GTY((skip)) union_ptr;  /* Pointer to union */
    void (*callback)(int);  /* Function pointer member */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *GTY((skip)) user_name;
    struct undefined_struct *GTY((skip)) undefined_ref;  /* Reference to undefined type */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *GTY((skip)) string_val;
    struct basic_struct *GTY((skip)) struct_val;
};

/* Internal union (not top-level GTY) */
union internal_union {
    int a;
    float b;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct struct_array_5[5];
typedef char *string_array[20];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((skip)) data;
    const char *GTY((skip)) const_data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*simple_callback_t)(void);
typedef struct basic_struct *(*factory_t)(int id);

/* Linked list structure (for traversal) */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
    void *GTY((skip)) data;
    int priority;
};

/* Tree structure (for complex traversal) */
struct GTY(()) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
    union data_union *GTY((skip)) value;
    enum color node_color;
};

/* Container with all types */
struct GTY(()) type_container {
    /* Scalars */
    int counter;
    double total;
    enum color default_color;
    
    /* Structs */
    struct basic_struct basic;
    struct complex_struct *GTY((skip)) complex_ptr;
    struct user_struct *GTY((skip)) user_struct_ptr;
    
    /* Unions */
    union data_union data;
    union internal_union internal;
    
    /* Pointers */
    int *int_ptr;
    void **void_ptr_ptr;
    struct list_node **node_ptr_array;
    
    /* Arrays */
    int numbers[100];
    struct basic_struct structs[10];
    char *strings[50];
    
    /* Strings */
    struct gcc_string title;
    struct gcc_string description;
    
    /* Callbacks */
    comparator_t compare_func;
    factory_t create_func;
    
    /* Collections */
    struct list_node *GTY((skip)) head;
    struct tree_node *GTY((skip)) root;
    
    /* Undefined reference */
    struct undefined_struct *GTY((skip)) undefined;
    
    /* Array of pointers */
    struct tree_node *GTY((skip)) node_ptrs[20];
    
    /* Multi-dimensional array */
    int matrix[5][5];
};

/* Root structure for gengtype traversal */
struct GTY(()) root_struct {
    struct type_container *GTY((skip)) container;
    struct list_node *GTY((skip)) global_list;
    struct tree_node *GTY((skip)) global_tree;
    struct gcc_string global_name;
    
    /* Array of containers */
    struct type_container containers[3];
    
    /* Pointer array with mixed types */
    void *GTY((skip)) mixed_ptrs[10];
};

/* Non-GTY struct (may be processed as TYPE_UNDEFINED when referenced) */
struct plain_struct {
    int x, y, z;
    struct undefined_struct *link;
};

/* Another callback type */
typedef void (*error_handler_t)(int code, const char *message);

/* Struct with callback array */
struct GTY(()) callback_container {
    error_handler_t handlers[5];
    simple_callback_t simple_handlers[10];
    comparator_t comparators[3];
};

#endif /* GTY_TEST_H */
