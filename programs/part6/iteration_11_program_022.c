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

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_helper {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    scalar_int_t id;
    char name[32];
    struct undefined_helper* helper;  /* Reference to non-GTY type */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* user_data;
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
    char* GTY((length("%0.length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func_t)(void* context, int value);
typedef void (*void_callback_t)(struct basic_struct*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    void* GTY((skip)) lang_data;  /* Skip for GC */
};

/* Complex nested structure with multiple type references */
struct GTY(()) complex_node {
    int data;
    enum color color;
    
    /* TYPE_POINTER */
    struct complex_node* next;
    struct complex_node* prev;
    
    /* TYPE_ARRAY */
    int values[8];
    
    /* TYPE_UNION */
    union data_union payload;
    
    /* TYPE_STRING */
    struct gcc_string* description;
    
    /* TYPE_CALLBACK */
    callback_func_t processor;
    
    /* TYPE_USER_STRUCT */
    struct user_struct* user_info;
    
    /* TYPE_LANG_STRUCT */
    struct lang_struct* lang_info;
    
    /* Array of pointers */
    struct basic_struct* GTY((length("%0.data"))) refs[4];
};

/* Linked list structure for deep traversal */
struct GTY(()) list_node {
    int value;
    struct GTY((tag("1"))) list_node* next;
    struct GTY((tag("0"))) list_node* child;
};

/* Root structure containing all types */
struct GTY(()) type_root {
    /* TYPE_STRUCT */
    struct basic_struct basic;
    
    /* Pointer to complex structure */
    struct complex_node* complex;
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_ARRAY */
    struct list_node* nodes[16];
    
    /* TYPE_STRING */
    struct gcc_string title;
    
    /* TYPE_CALLBACK */
    void_callback_t callback;
    
    /* TYPE_USER_STRUCT */
    struct user_struct* user;
    
    /* TYPE_LANG_STRUCT */
    struct lang_struct* lang;
    
    /* Scalar members */
    scalar_int_t count;
    scalar_double_t total;
    enum color default_color;
    
    /* Various pointers */
    int_ptr_t int_buffer;
    generic_ptr_t generic_data;
    
    /* Fixed-size array */
    int_array_10 fixed_array;
};

/* External declaration to force inclusion in type graph */
extern struct type_root GTY((extern)) global_root;

#endif /* GTY_TEST_H */
