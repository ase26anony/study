/* gtype-test.h - Test header for gengtype coverage */
#ifndef GTYPE_TEST_H
#define GTYPE_TEST_H

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

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) basic_ptr;
    int count;
    struct undefined_helper *helper;  /* Will be TYPE_UNDEFINED */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *GTY((skip)) opaque_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    double double_val;
    char *GTY((tag("STRING"))) string_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void *void_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct struct_array_5[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *data, int param);
typedef void (*simple_callback_t)(void);

/* Struct with callback member */
struct GTY(()) callback_container {
    callback_func_t GTY((skip)) handler;
    void *GTY((skip)) user_data;
    int state;
};

/* Linked list structure for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *GTY((skip)) next;
    struct list_node *GTY((skip)) prev;
};

/* Tree structure for complex traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct gcc_string *GTY((skip)) label;
};

/* Array of pointers */
struct GTY(()) pointer_array {
    void *GTY((skip)) pointers[8];
    int count;
};

/* Mixed type container */
struct GTY(()) mixed_container {
    union data_union *GTY((skip)) union_ptr;
    struct user_struct *GTY((skip)) user_struct_ptr;
    callback_func_t GTY((skip)) callback;
    int_array_10 numbers;
    struct gcc_string description;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_tag;
    void *GTY((skip)) lang_data;
    struct GTY((skip)) lang_struct *next;
};

/* Root structure containing pointers to everything */
struct GTY(()) root_container {
    /* Struct types */
    struct basic_struct *GTY((skip)) basic;
    struct complex_struct *GTY((skip)) complex;
    
    /* Union type */
    union data_union *GTY((skip)) data_union_ptr;
    
    /* User struct */
    struct user_struct *GTY((skip)) user;
    
    /* String type */
    struct gcc_string *GTY((skip)) string;
    
    /* Callback container */
    struct callback_container *GTY((skip)) callback;
    
    /* List structure */
    struct list_node *GTY((skip)) list_head;
    
    /* Tree structure */
    struct tree_node *GTY((skip)) tree_root;
    
    /* Array of pointers */
    struct pointer_array *GTY((skip)) ptr_array;
    
    /* Mixed container */
    struct mixed_container *GTY((skip)) mixed;
    
    /* Lang struct */
    struct lang_struct *GTY((skip)) lang;
    
    /* Direct pointers */
    int_ptr_t int_ptr;
    void_ptr_t void_ptr;
    
    /* Arrays */
    int_array_10 direct_array;
    struct_array_5 struct_array;
    
    /* Scalar members */
    scalar_int_t scalar_int;
    scalar_double_t scalar_double;
    enum color color;
    
    /* Callback function pointer */
    callback_func_t direct_callback;
};

/* External declaration for gengtype to process */
extern struct root_container GTY((root)) global_root;

#endif /* GTYPE_TEST_H */
