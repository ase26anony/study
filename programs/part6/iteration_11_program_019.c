/* Test header for gengtype coverage testing */
#ifndef GTYPE_TEST_H
#define GTYPE_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef char scalar_char_t;

/* TYPE_ENUM (treated as scalar) */
enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_UNDEFINED: Type without GTY marker but referenced by annotated types */
struct undefined_struct {
    int x;
    double y;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    double value;
    char *description;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *base;  /* TYPE_POINTER */
    int scores[10];             /* TYPE_ARRAY within struct */
    enum color color;
    struct undefined_struct *undef;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;  /* Untraced pointer */
};

/* TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;
    struct basic_struct *struct_ptr;
};

/* TYPE_ARRAY: Typedef for array type */
typedef int GTY(()) int_array_t[100];
typedef struct basic_struct * GTY(()) struct_ptr_array_t[50];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void *generic_ptr_t;

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char * GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *data, int param);
typedef void (*cleanup_func_t)(struct basic_struct *obj);

/* Struct with callback member */
struct GTY(()) callback_container {
    callback_func_t callback;
    cleanup_func_t cleanup;
    void *user_data;
};

/* Linked list structure (chain of types) */
struct GTY(()) list_node {
    int data;
    struct list_node *next;
    struct list_node *prev;
};

/* Tree structure with multiple pointer types */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
    union data_union node_data;
};

/* Container with array of pointers */
struct GTY(()) pointer_array_container {
    int count;
    struct basic_struct *items[20];
    struct complex_struct *complex_items[10];
};

/* Mixed type container */
struct GTY(()) mixed_container {
    /* Scalars */
    int id;
    double score;
    enum color bg_color;
    
    /* Pointers */
    struct basic_struct *base;
    struct complex_struct *complex;
    struct user_struct *user;
    
    /* Arrays */
    int numbers[5];
    struct basic_struct *ptr_array[3];
    
    /* Union */
    union data_union storage;
    
    /* String */
    struct gcc_string *name;
    
    /* Callback */
    callback_func_t handler;
    
    /* Linked structure */
    struct list_node *head;
    
    /* Tree structure */
    struct tree_node *root;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Root structure containing pointers to all types */
struct GTY(()) gtype_root {
    /* Basic types */
    struct basic_struct *basic;
    struct complex_struct *complex;
    
    /* Special types */
    struct user_struct *user;
    union data_union *union_data;
    struct gcc_string *string_data;
    
    /* Containers */
    struct callback_container *callback;
    struct pointer_array_container *ptr_array;
    struct mixed_container *mixed;
    
    /* Linked structures */
    struct list_node *list;
    struct tree_node *tree;
    
    /* Arrays */
    int_array_t *int_array;
    struct_ptr_array_t *struct_array;
    
    /* Various pointers */
    basic_ptr_t basic_ptr;
    int_ptr_t int_ptr;
    generic_ptr_t generic_ptr;
    
    /* Callback */
    callback_func_t root_callback;
};

/* External declaration for gengtype to process */
extern struct gtype_root * GTY(()) global_root;

#endif /* GTYPE_TEST_H */
