/* test_types.h - Comprehensive GTY-annotated types for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro definitions */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Regular struct with various members */
struct GTY((chain_next ("next"), chain_prev ("prev"))) linked_node {
    int GTY(()) value;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
    
    /* Anonymous struct member */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } point;
    
    /* Bit fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
    
    /* Nested struct */
    struct GTY(()) inner {
        int GTY(()) depth;
        char GTY(()) label[20];
    } GTY(()) inner_data;
};

/* Another struct for TYPE_STRUCT */
struct GTY(()) complex_struct {
    scalar_int GTY(()) id;
    scalar_float GTY(()) weight;
    struct linked_node * GTY(()) head;
    
    /* Anonymous union within struct */
    union GTY(()) {
        int GTY(()) int_val;
        float GTY(()) float_val;
    } GTY(()) data;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int GTY(()) custom_field;
    char * GTY(()) custom_name;
};

/* TYPE_UNION: Standalone union type */
union GTY(()) data_container {
    int GTY(()) int_data;
    float GTY(()) float_data;
    double GTY(()) double_data;
    char * GTY(()) string_data;
    struct linked_node * GTY(()) node_data;
};

/* Tagged union with discriminator */
struct GTY(()) tagged_union_container {
    enum GTY(()) { INT_TYPE, FLOAT_TYPE, STRING_TYPE } GTY(()) type;
    union GTY(()) {
        int GTY(()) int_val;
        float GTY(()) float_val;
        char * GTY((length ("strlen(%h) + 1"))) str_val;
    } GTY(()) data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct linked_node * GTY(()) node_ptr;
typedef struct complex_struct * GTY(()) complex_ptr;
typedef union data_container * GTY(()) container_ptr;
typedef void * GTY(()) generic_ptr;
typedef node_ptr * GTY(()) ptr_to_ptr;

/* Function pointer type for TYPE_CALLBACK */
typedef int GTY(()) (*compare_func)(const void *, const void *);
typedef void GTY(()) (*callback_func)(int, char *);

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct linked_node GTY(()) node_array[5];
typedef union data_container GTY(()) container_array[3][3];
typedef char * GTY(()) string_array[8];

/* Struct containing arrays */
struct GTY(()) array_container {
    int GTY(()) matrix[4][4];
    struct linked_node GTY(()) nodes[10];
    char GTY(()) buffer[256];
    color_t GTY(()) colors[6];
};

/* TYPE_STRING: String types */
typedef char * GTY((length ("strlen(%h) + 1"))) gty_string;
typedef const char * GTY((length ("strlen(%h) + 1"))) const_gty_string;

/* Struct with string members */
struct GTY(()) string_container {
    char * GTY((length ("strlen(%h.name) + 1"))) name;
    const char * GTY((length ("strlen(%h.title) + 1"))) title;
    gty_string GTY(()) dynamic_str;
};

/* TYPE_CALLBACK: Struct with callback members */
struct GTY(()) callback_container {
    compare_func GTY(()) compare;
    callback_func GTY(()) notify;
    void (* GTY(()) raw_func_ptr)(void);
    
    /* Callback in union */
    union GTY(()) {
        compare_func GTY(()) cmp;
        callback_func GTY(()) cb;
    } GTY(()) func_union;
};

/* Complex type with all kinds mixed */
struct GTY((desc ("%0.tag"))) master_type {
    enum GTY(()) { 
        MASTER_SCALAR, 
        MASTER_STRUCT, 
        MASTER_UNION, 
        MASTER_ARRAY 
    } GTY(()) tag;
    
    union GTY(()) {
        /* Scalar in union */
        scalar_double GTY(()) scalar_val;
        
        /* Pointer in union */
        struct complex_struct * GTY(()) struct_ptr;
        
        /* Array in union */
        int GTY(()) int_arr[5];
        
        /* String in union */
        char * GTY((length ("strlen(%h) + 1"))) str_val;
    } GTY(()) data;
    
    /* Callback member */
    compare_func GTY(()) sorter;
    
    /* Array of pointers */
    struct linked_node * GTY(()) node_ptrs[8];
    
    /* Multi-dimensional array */
    color_t GTY(()) color_grid[3][3];
};

/* Global variable declarations */
extern struct linked_node GTY(()) global_node;
extern struct complex_struct GTY(()) global_complex;
extern union data_container GTY(()) global_container;
extern struct array_container GTY(()) global_arrays;
extern struct string_container GTY(()) global_strings;
extern struct callback_container GTY(()) global_callbacks;
extern struct master_type GTY(()) global_master;

#endif /* TEST_TYPES_H */
