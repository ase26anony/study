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
    int scores[10];             /* TYPE_ARRAY within struct */
    enum color color;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    void *user_data;  /* Untraced pointer */
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;  /* TYPE_POINTER within union */
    void *ptr_val;
};

/* TYPE_ARRAY: Typedef for array type */
typedef int int_array_t[100];
typedef struct basic_struct* struct_ptr_array_t[50];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;  /* TYPE_POINTER to char */
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void *context, int value);
typedef void (*simple_callback_t)(void);

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t;
typedef union data_union *union_ptr_t;
typedef callback_func_t callback_ptr_t;

/* Linked list structure for TYPE_STRUCT with TYPE_POINTER chain */
struct GTY(()) linked_node {
    int data;
    struct linked_node *next;  /* Self-referential pointer */
    struct linked_node *prev;
};

/* Container with multiple type references */
struct GTY(()) type_container {
    /* TYPE_STRUCT members */
    struct basic_struct basic;
    struct complex_struct *complex_ptr;
    
    /* TYPE_UNION member */
    union data_union data;
    
    /* TYPE_ARRAY members */
    int numbers[20];
    struct linked_node *node_array[5];
    
    /* TYPE_STRING member */
    struct gcc_string description;
    
    /* TYPE_CALLBACK member */
    callback_func_t callback;
    
    /* TYPE_POINTER members */
    void *opaque_ptr;
    int *int_ptr;
    char **string_array;  /* Pointer to pointer */
    
    /* TYPE_SCALAR members */
    enum color bg_color;
    long counter;
    float ratio;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_struct {
    int lang_specific;
    void *lang_data;
};
#endif

/* Root structure containing all types */
struct GTY(()) root_container {
    struct type_container *main_container;
    struct linked_node *node_list;
    union data_union *union_ptr;
    struct gcc_string *title;
    callback_func_t handlers[3];  /* Array of callbacks */
    
    /* Mix of annotated and non-annotated types */
    struct undefined_struct *undefined;  /* Will be TYPE_UNDEFINED */
    struct user_struct *user;           /* TYPE_USER_STRUCT */
    
    /* Array of various pointers */
    void *ptr_array[10];
};

/* Non-GTY struct that will be referenced */
struct referenced_but_not_gty {
    int x;
    int y;
    struct basic_struct *gty_ptr;  /* Points to GTY type */
};

/* Another GTY struct referencing non-GTY struct */
struct GTY(()) mixed_references {
    struct referenced_but_not_gty *non_gty_ptr;  /* TYPE_POINTER */
    int flags;
};

#endif /* GTY_TEST_H */
