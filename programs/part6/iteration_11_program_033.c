/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRUCT: Basic struct without GTY marker */
struct unannotated_struct {
    int id;
    char name[32];
};

/* TYPE_STRUCT with GTY marker */
struct GTY(()) base_struct {
    int counter;
    double value;
    enum color color;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void* GTY((skip)) opaque_data;
    int user_id;
};

/* TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char* string_val;
    void* ptr_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* base_ptr_t;
typedef int* int_ptr_t;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct base_struct* ptr_array_5[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char* GTY((length("((gcc_string*)&_)->length"))) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void* context, int param);

/* Another struct using callback */
struct GTY(()) callback_container {
    callback_func_t handler;
    void* user_data;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_node {
    int id;
    
    /* TYPE_POINTER to same type (linked list) */
    struct complex_node* GTY((tag("0"))) next;
    
    /* TYPE_UNION */
    union GTY(()) node_data {
        int int_data;
        double double_data;
        struct gcc_string* string_data;
    } data;
    
    /* TYPE_ARRAY */
    int scores[5];
    
    /* TYPE_POINTER array */
    struct base_struct* references[3];
    
    /* TYPE_CALLBACK */
    callback_func_t notify;
};

/* Container with multiple type references */
struct GTY(()) type_container {
    /* TYPE_STRUCT reference */
    struct base_struct base;
    
    /* TYPE_USER_STRUCT reference */
    struct user_struct* user_data;
    
    /* TYPE_UNION */
    union data_union storage;
    
    /* TYPE_POINTER to unannotated type */
    struct unannotated_struct* unannotated;
    
    /* TYPE_ARRAY of structs */
    struct complex_node nodes[4];
    
    /* TYPE_STRING */
    struct gcc_string description;
    
    /* TYPE_CALLBACK */
    callback_func_t callback;
    
    /* TYPE_POINTER to array */
    int* dynamic_array;
    
    /* TYPE_SCALAR */
    long timestamp;
};

/* Root structure containing everything */
struct GTY(()) root_struct {
    struct type_container* container;
    struct complex_node* node_list;
    union data_union root_union;
    struct gcc_string root_string;
    callback_func_t root_callback;
    
    /* Array of pointers to different types */
    void* GTY((desc("1"))) polymorphic_array[8];
};

/* External variable to ensure types are used */
extern struct root_struct GTY((root)) global_root;

#endif /* GTY_TEST_H */
