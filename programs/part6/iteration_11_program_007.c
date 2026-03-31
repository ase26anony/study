/* Test header for gengtype coverage */
#ifndef GTYPE_TEST_H
#define GTYPE_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_t;

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_helper {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic GTY struct */
struct GTY(()) base_struct {
    int id;
    char name[32];
    double value;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct base_struct *base;  /* TYPE_POINTER */
    int scores[10];            /* TYPE_ARRAY within struct */
    struct undefined_helper *helper; /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: GTY union */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;          /* TYPE_STRING pointer */
    struct base_struct *struct_ptr;
};

/* TYPE_ARRAY: Typedef for array type */
typedef int GTY(()) int_array_t[50];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;                /* Flexible string data */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int GTY(()) (*callback_func)(void *context, int value);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func handler;
    void *context;
    int threshold;
};

/* Linked list for chained references (triggers deep traversal) */
struct GTY(()) list_node {
    int data;
    struct list_node *next;    /* Self-referential pointer */
    struct list_node *prev;    /* Another pointer for complexity */
    union data_union node_data; /* Embedded union */
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct base_struct *items[20];     /* Fixed array of pointers */
    struct list_node **dynamic_items;  /* Pointer to pointer array */
    int count;
};

/* Nested struct with all types */
struct GTY(()) master_container {
    /* Scalars */
    int id;
    color_t color;
    
    /* Pointers to various types */
    struct base_struct *base_ptr;
    struct complex_struct *complex_ptr;
    union data_union *union_ptr;
    struct gcc_string *string_ptr;
    
    /* Arrays */
    int_array_t fixed_array;
    int dynamic_array[30];
    
    /* Embedded structs/unions */
    struct base_struct embedded_struct;
    union data_union embedded_union;
    
    /* Callback */
    callback_func notify;
    
    /* Complex nested */
    struct list_node *list_head;
    struct pointer_array *ptr_array;
    
    /* User struct */
    struct user_struct *user_data;
    
    /* Undefined type reference */
    struct undefined_helper *undefined_ref;
};

/* Root structure containing everything */
struct GTY(()) root_type {
    struct master_container *container;
    struct list_node *global_list;
    struct gcc_string *title;
    callback_func global_handler;
    union data_union root_data;
    int_array_t root_array;
};

/* Additional non-GTY types that might be referenced */
typedef struct {
    int x, y;
} point_t;

struct another_undefined {
    point_t position;
    char description[100];
};

#endif /* GTYPE_TEST_H */
