/* Header file to trigger gengtype state writing functions */
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

/* TYPE_UNDEFINED: Type without GTY marker but referenced from GTY types */
struct undefined_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
    void* user_data;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char* string_val;
    void* ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct* ptr_array_5[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char* GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func)(void* context, int value);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    union data_union lang_data;
    struct gcc_string* lang_name;
};

/* Complex nested structures to ensure deep traversal */

/* Linked list node (TYPE_STRUCT with TYPE_POINTER member) */
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
    struct gcc_string* label;
    callback_func on_visit;
};

/* Container with array of pointers */
struct GTY(()) pointer_container {
    int count;
    void* GTY((skip)) pointers[8];
    struct basic_struct* items[4];
};

/* Union container */
struct GTY(()) union_container {
    int type;
    union data_union data;
    callback_func processor;
};

/* Mixed type structure */
struct GTY(()) mixed_struct {
    /* TYPE_SCALAR members */
    int id;
    enum color color;
    double weight;
    
    /* TYPE_POINTER members */
    struct basic_struct* base;
    int* numbers;
    char* text;
    
    /* TYPE_ARRAY members */
    int scores[5];
    struct basic_struct* objects[3];
    
    /* TYPE_UNION member */
    union data_union variant;
    
    /* TYPE_STRING member */
    struct gcc_string description;
    
    /* TYPE_CALLBACK member */
    callback_func handler;
    
    /* Reference to TYPE_UNDEFINED */
    struct undefined_struct* undefined_ref;
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* TYPE_STRUCT references */
    struct basic_struct* basic;
    struct user_struct* user;
    struct mixed_struct* mixed;
    
    /* TYPE_UNION reference */
    union data_union* union_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct tree_node* nodes[10];
    
    /* TYPE_STRING reference */
    struct gcc_string* title;
    
    /* TYPE_LANG_STRUCT reference */
    struct lang_struct* lang;
    
    /* TYPE_CALLBACK array */
    callback_func callbacks[5];
    
    /* Linked list */
    struct list_node* head;
    
    /* Tree */
    struct tree_node* root;
    
    /* Containers */
    struct pointer_container* ptr_container;
    struct union_container* union_container;
    
    /* Scalar values */
    scalar_int_t counter;
    scalar_double_t total;
    
    /* Direct arrays */
    int direct_array[20];
    struct basic_struct direct_structs[5];
};

/* Global root variable */
extern struct root_container GTY((root)) global_root;

/* Function pointer type definitions for callbacks */
typedef struct GTY(()) callback_wrapper {
    char* name;
    callback_func func;
    void* user_data;
} callback_wrapper_t;

/* Another complex structure with nested arrays */
struct GTY(()) matrix_container {
    int rows;
    int cols;
    double* GTY((length("%0.rows * %0.cols"))) data;
    struct gcc_string* row_labels;
    struct gcc_string* col_labels;
};

/* Structure with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

#endif /* GTY_TEST_H */
