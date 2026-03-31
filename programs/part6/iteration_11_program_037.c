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

/* TYPE_UNDEFINED: Type without GTY marker (will be referenced from GTY types) */
struct undefined_struct {
    int id;
    char *name;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char *name;
    double value;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((skip)) ptr_to_basic;
    int count;
    enum color color;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *user_data;
    void *GTY((skip)) opaque_ptr;
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
typedef struct basic_struct *struct_ptr_array[5];

/* Fixed-size array within struct */
struct GTY(()) array_container {
    int id;
    int numbers[20];
    struct basic_struct *objects[8];
};

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char *GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(struct basic_struct *, int, char *);

struct GTY(()) callback_container {
    simple_callback_t GTY((skip)) simple_cb;
    complex_callback_t GTY((skip)) complex_cb;
    int (*inline_cb)(double, float);
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    union data_union *data;
};

/* Linked list structure for chained references */
struct GTY(()) linked_node {
    int data;
    struct linked_node *next;
    struct linked_node *prev;
};

/* Tree structure for complex graph */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Container with mixed types */
struct GTY(()) mixed_container {
    /* Scalar members */
    int id;
    enum color color;
    double weight;
    
    /* Pointer members */
    struct basic_struct *basic;
    struct user_struct *user;
    struct undefined_struct *undefined;  /* Will trigger TYPE_UNDEFINED */
    
    /* Union member */
    union data_union data;
    
    /* Array members */
    int scores[5];
    struct basic_struct *items[3];
    
    /* String member */
    struct gcc_string name;
    
    /* Callback member */
    simple_callback_t callback;
    
    /* Linked structure */
    struct linked_node *list_head;
    
    /* Tree structure */
    struct tree_node *tree_root;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct mixed_container main;
    struct array_container arrays;
    struct callback_container callbacks;
    struct lang_struct lang;
    
    /* Direct pointers to various types */
    struct basic_struct *basic_ptr;
    struct user_struct *user_ptr;
    union data_union *union_ptr;
    struct gcc_string *string_ptr;
    struct linked_node *list_ptr;
    struct tree_node *tree_ptr;
    
    /* Arrays of different types */
    struct basic_struct *basic_array[4];
    union data_union union_array[3];
    struct gcc_string string_array[2];
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* External declaration to force type inclusion */
extern struct root_container * GTY((tag("ROOT"))) global_root;

#endif /* GTY_TEST_H */
