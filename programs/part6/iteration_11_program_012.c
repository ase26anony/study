/* gtype-test.h - Test header for gengtype coverage */
#ifndef GTYPE_TEST_H
#define GTYPE_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_t;

/* TYPE_UNDEFINED: Non-GTY annotated struct (will be referenced from GTY types) */
struct undefined_struct {
    int x;
    char y;
};

/* TYPE_STRUCT: Basic GTY-annotated struct */
struct GTY(()) base_struct {
    int id;
    char* name;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct base_struct* GTY((skip)) base;  /* TYPE_POINTER */
    int GTY((tag)) values[5];              /* TYPE_ARRAY within struct */
    struct undefined_struct* undefined_ptr; /* Pointer to non-GTY type */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* GTY((skip)) user_data;
    int (* GTY((skip)) compare_func)(const void*, const void*);
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char* GTY((skip)) string_val;
    struct base_struct* GTY((skip)) struct_ptr;
};

/* TYPE_ARRAY: Array typedef */
typedef int GTY(()) int_array_t[10];
typedef struct base_struct* GTY(()) struct_ptr_array_t[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char* GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (* GTY((skip)) callback_func_t)(void* context, int value);

/* Struct using callback type */
struct GTY(()) callback_container {
    callback_func_t GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Linked list structure for chained references */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((skip)) next;
    struct list_node* GTY((skip)) prev;
};

/* Tree node with multiple pointer types */
struct GTY(()) tree_node {
    int value;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
    union data_union GTY((skip)) node_data;
};

/* Container with array of pointers */
struct GTY(()) pointer_container {
    struct base_struct* GTY((skip)) items[8];      /* Array of pointers */
    struct gcc_string* GTY((skip)) strings[4];     /* Array of string pointers */
    callback_func_t GTY((skip)) callbacks[3];      /* Array of callbacks */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific_flag;
    void* GTY((skip)) lang_data;
};

/* Root structure containing pointers to all other types */
struct GTY(()) root_container {
    struct base_struct* GTY((skip)) base_ptr;
    struct complex_struct* GTY((skip)) complex_ptr;
    struct user_struct* GTY((skip)) user_ptr;
    union data_union GTY((skip)) union_data;
    struct gcc_string* GTY((skip)) string_ptr;
    struct callback_container* GTY((skip)) callback_ptr;
    struct list_node* GTY((skip)) list_head;
    struct tree_node* GTY((skip)) tree_root;
    struct pointer_container* GTY((skip)) ptr_container;
    struct lang_struct* GTY((skip)) lang_ptr;
    
    /* Direct scalar members */
    scalar_int_t direct_int;
    scalar_double_t direct_double;
    color_t color;
    
    /* Arrays */
    int_array_t int_array;
    struct_ptr_array_t struct_array;
    
    /* Callback */
    callback_func_t direct_callback;
};

/* Additional pointer types */
typedef struct root_container* GTY((skip)) root_ptr_t;
typedef struct list_node* GTY((skip)) node_ptr_t;

/* Mixed declaration styles */
typedef struct GTY(()) {
    int anonymous_id;
    char* GTY((skip)) anonymous_name;
} anonymous_struct_t;

#endif /* GTYPE_TEST_H */
