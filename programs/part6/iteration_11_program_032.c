/* Test header for gengtype coverage testing */
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
    struct basic_struct *base;  /* TYPE_POINTER */
    int scores[10];             /* TYPE_ARRAY of scalar */
    struct complex_struct *next; /* Linked list pointer */
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char *user_name;           /* TYPE_STRING pointer */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;          /* TYPE_STRING pointer */
    void *ptr_val;             /* TYPE_POINTER */
};

/* TYPE_ARRAY: Typedef for array type */
typedef int int_array_t[100];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;                /* Flexible string buffer */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *context, int value);

/* TYPE_CALLBACK in struct */
struct GTY(()) callback_container {
    callback_func_t handler;
    void *user_data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Linked list for traversal */
struct GTY(()) list_node {
    int data;
    struct list_node *prev;
    struct list_node *next;
    union data_union node_data;  /* TYPE_UNION member */
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct basic_struct *items[20];  /* TYPE_ARRAY of TYPE_POINTER */
    void *generic_ptrs[10];
};

/* Mixed type container */
struct GTY(()) mixed_container {
    /* Scalar types */
    int counter;
    enum color current_color;
    
    /* Pointer types */
    struct complex_struct *complex;
    struct user_struct *user;
    
    /* Array types */
    int matrix[3][3];           /* Multi-dimensional array */
    struct list_node *nodes[5]; /* Array of pointers */
    
    /* Union */
    union data_union storage;
    
    /* String */
    struct gcc_string *title;
    
    /* Callback */
    callback_func_t notify;
    
    /* Undefined reference (TYPE_UNDEFINED) */
    struct undefined_struct *undefined;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_field;
    struct GTY((skip)) {
        int skipped_field;
    } skip_section;
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct basic_struct basic;
    struct complex_struct *complex_chain;
    struct user_struct user_obj;
    union data_union main_data;
    struct gcc_string main_string;
    struct callback_container callback;
    struct list_node *list_head;
    struct pointer_array ptr_array;
    struct mixed_container mixed;
    struct lang_struct lang;
    
    /* Array of different types */
    struct basic_struct basic_array[5];
    union data_union union_array[3];
    callback_func_t callbacks[4];
};

/* Non-GTY struct that will be referenced */
struct referenced_without_gty {
    int ref_id;
    char ref_name[50];
};

/* Another GTY struct referencing non-GTY struct */
struct GTY(()) has_non_gty_reference {
    struct referenced_without_gty *ref;  /* Will be TYPE_UNDEFINED in gengtype */
    int value;
};

#endif /* GTYPE_TEST_H */
