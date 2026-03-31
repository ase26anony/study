/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef float GTY(()) scalar_float;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    double GTY(()) value;
    enum color GTY(()) color;
};

/* TYPE_STRUCT with nested anonymous struct */
struct GTY(()) complex_struct {
    int GTY(()) tag;
    union {
        int GTY(()) int_val;
        double GTY(()) double_val;
    } GTY(()) data;
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) : 4;  /* Unnamed bitfield */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    void* GTY((skip)) user_data;
    int GTY(()) user_id;
};

/* TYPE_UNION: Basic union type */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY((length(strlen))) string_val;
    struct basic_struct* GTY(()) struct_ptr;
};

/* TYPE_UNION with tag */
struct GTY(()) tagged_union_container {
    int GTY(()) type_tag;
    union {
        int GTY(()) as_int;
        double GTY(()) as_double;
        struct basic_struct GTY(()) as_struct;
    } GTY(()) data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_pointer;
typedef union data_union* GTY(()) union_pointer;
typedef void (*GTY(()) func_ptr)(int, double);
typedef void* GTY(()) void_pointer;
typedef int** GTY(()) pointer_to_pointer;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[3][3];
typedef char* GTY(()) string_array[8];

/* TYPE_STRING: String types */
typedef char* GTY((length(strlen))) gty_string;
typedef const char* GTY((length(strlen))) const_gty_string;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    compare_func GTY(()) sorter;
    void* GTY(()) data;
    size_t GTY(()) size;
};

/* Linked list structure for chain_next/chain_prev testing */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int GTY(()) value;
    struct linked_node* GTY(()) next;
    struct linked_node* GTY(()) prev;
};

/* Variable length array structure */
struct GTY(()) var_len_struct {
    int GTY(()) count;
    int GTY((length("%h.count"))) items[1];
};

/* Nested type references creating a web of dependencies */
struct GTY(()) master_container {
    /* TYPE_STRUCT */
    struct basic_struct GTY(()) basic;
    
    /* TYPE_UNION */
    union data_union GTY(()) data;
    
    /* TYPE_POINTER */
    struct_pointer GTY(()) struct_ptr;
    union_pointer GTY(()) union_ptr;
    void_pointer GTY(()) generic_ptr;
    
    /* TYPE_ARRAY */
    int_array GTY(()) numbers;
    struct_array GTY(()) objects;
    
    /* TYPE_SCALAR */
    scalar_int GTY(()) id;
    scalar_double GTY(()) weight;
    color_t GTY(()) color;
    
    /* TYPE_STRING */
    gty_string GTY(()) name;
    const_gty_string GTY(()) description;
    
    /* TYPE_CALLBACK */
    callback_container GTY(()) sorter;
    
    /* TYPE_UNDEFINED pointer */
    struct opaque_struct* GTY(()) opaque;
    
    /* Linked list */
    struct linked_node* GTY(()) list_head;
    
    /* Variable length */
    struct var_len_struct* GTY(()) var_data;
};

/* Global variable declarations */
extern struct master_container GTY(()) global_container;
extern struct linked_node* GTY(()) global_list;
extern gty_string GTY(()) global_strings[];

#endif /* TEST_TYPES_H */
