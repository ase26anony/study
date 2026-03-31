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

/* TYPE_UNDEFINED: Non-GTY annotated struct referenced by GTY types */
struct undefined_helper {
    int internal_data;
    void* opaque_ptr;
};

/* TYPE_STRUCT: Basic GTY struct */
struct GTY(()) basic_struct {
    int id;
    char name[32];
    double value;
};

/* TYPE_STRUCT with nested references */
struct GTY(()) complex_struct {
    struct basic_struct *GTY((tag("BASIC"))) base;
    struct undefined_helper *helper;  /* Will be TYPE_UNDEFINED */
    enum color color;
    long timestamp;
};

/* TYPE_USER_STRUCT: User-defined structure */
struct user_data {
    int user_id;
    char* username;
};

typedef struct user_data user_data_t;

struct GTY((user)) user_wrapper {
    user_data_t* GTY((skip)) data;  /* User type, skip for GC */
    int ref_count;
};

/* TYPE_UNION */
union GTY(()) variant_data {
    int int_val;
    double double_val;
    char* GTY((tag("STRING"))) string_val;
    struct basic_struct* GTY((tag("STRUCT"))) struct_val;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* basic_ptr_t;
typedef int* int_ptr_t;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array_10[10];
typedef struct basic_struct* struct_ptr_array[5];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    int capacity;
    char* GTY((length("%h.length + 1"))) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*cleanup_callback)(void* user_data);

/* Linked list structure to create type chains */
struct GTY(()) list_node {
    int id;
    char* GTY((tag("STRING"))) label;
    struct list_node* GTY((tag("NEXT"))) next;
    struct list_node* GTY((tag("PREV"))) prev;
    union variant_data GTY((tag("DATA"))) data;
};

/* Array of pointers in a struct */
struct GTY(()) pointer_container {
    struct basic_struct* GTY((tag("BASIC_PTR"))) basic_ptrs[4];
    struct list_node** GTY((tag("NODE_PTRS"))) node_ptr_array;
    int ptr_count;
};

/* Nested structure with callback */
struct GTY(()) callback_container {
    cleanup_callback GTY((tag("CALLBACK"))) cleanup;
    void* GTY((tag("USER_DATA"))) user_data;
    comparator_t compare;
};

/* Root structure containing all types */
struct GTY(()) root_container {
    /* TYPE_STRUCT references */
    struct basic_struct GTY((tag("BASIC"))) basic;
    struct complex_struct* GTY((tag("COMPLEX"))) complex;
    
    /* TYPE_UNION */
    union variant_data GTY((tag("VARIANT"))) variant;
    
    /* TYPE_POINTER */
    basic_ptr_t GTY((tag("BASIC_PTR"))) basic_ptr;
    int_ptr_t int_ptr;
    
    /* TYPE_ARRAY */
    int_array_10 GTY((tag("INT_ARRAY"))) numbers;
    struct_ptr_array GTY((tag("STRUCT_ARRAY"))) struct_ptrs;
    
    /* TYPE_STRING */
    struct gcc_string* GTY((tag("STRING"))) message;
    
    /* TYPE_USER_STRUCT */
    struct user_wrapper GTY((tag("USER_WRAPPER"))) user_wrap;
    
    /* TYPE_CALLBACK */
    callback_container* GTY((tag("CALLBACK_CONT"))) callbacks[3];
    
    /* Linked list */
    struct list_node* GTY((tag("LIST_HEAD"))) head;
    struct list_node* GTY((tag("LIST_TAIL"))) tail;
    
    /* Pointer container */
    struct pointer_container* GTY((tag("PTR_CONT"))) ptr_cont;
    
    /* Scalar types */
    scalar_int_t counter;
    scalar_double_t total;
    enum color current_color;
};

/* Additional TYPE_ARRAY as standalone typedef */
typedef struct list_node* node_array_t[8];

/* Function pointer array */
typedef cleanup_callback callback_array_t[5];

#endif /* GTY_TEST_H */
