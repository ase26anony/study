/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_UNDEFINED: Type without GTY marker, referenced by annotated types */
struct undefined_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) basic_struct {
    int id;
    char tag;
    scalar_double_t value;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct GTY((user)) user_struct {
    int user_id;
    char* user_name;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    char* string_val;
    scalar_double_t double_val;
};

/* TYPE_POINTER: Various pointer types */
typedef basic_struct* struct_ptr_t;
typedef int* int_ptr_t;
typedef void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct_ptr_t ptr_array_5[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int GTY((skip)) length;  /* skip for GC */
    char* GTY((length("length"))) data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_func)(void* context, int value);
typedef void (*simple_callback)(void);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
    int lang_specific;
    void* lang_data;
};

/* Complex nested structures to ensure deep traversal */

/* Linked list node (TYPE_STRUCT with TYPE_POINTER member) */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((skip)) next;  /* Skip to avoid infinite recursion in test */
    callback_func processor;
};

/* Container with multiple type references */
struct GTY(()) type_container {
    /* TYPE_STRUCT reference */
    basic_struct basic;
    
    /* TYPE_UNION reference */
    data_union union_data;
    
    /* TYPE_POINTER references */
    struct undefined_struct* undefined_ptr;
    user_struct* user_ptr;
    
    /* TYPE_ARRAY members */
    int numbers[20];
    struct_ptr_t pointers[8];
    
    /* TYPE_STRING reference */
    struct gcc_string* str;
    
    /* TYPE_CALLBACK member */
    callback_func callback;
    
    /* TYPE_SCALAR members */
    scalar_int_t count;
    enum color color;
    
    /* TYPE_LANG_STRUCT reference */
    struct lang_struct* lang;
    
    /* Nested anonymous union */
    union {
        int as_int;
        float as_float;
    } GTY((tag("0"))) anonymous_union;
};

/* Array of containers */
typedef type_container container_array[4];

/* Root structure containing all types */
struct GTY(()) root_struct {
    /* Direct type instances */
    basic_struct basic_instance;
    user_struct user_instance;
    data_union union_instance;
    struct gcc_string string_instance;
    
    /* Pointers to various types */
    struct undefined_struct* undefined_ref;
    list_node* list_head;
    container_array* containers;
    
    /* Function pointer table */
    callback_func callbacks[3];
    
    /* Mixed array */
    void* mixed_array[10];
    
    /* Reference to lang struct */
    struct lang_struct lang_instance;
    
    /* Scalar fields */
    scalar_int_t total_count;
    scalar_double_t total_value;
    enum color primary_color;
};

/* Global root pointer for GC */
extern struct root_struct* GTY((root)) global_root;

/* Function pointer typedefs for more callback coverage */
typedef void (*traversal_callback)(struct root_struct* root);
typedef int (*comparison_callback)(const void*, const void*);

/* Another struct with callback array */
struct GTY(()) callback_container {
    simple_callback init;
    callback_func process;
    traversal_callback traverse;
    comparison_callback compare;
};

#endif /* GTY_TEST_H */
