/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY headers */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
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

/* TYPE_STRUCT: Regular struct with nested members */
struct GTY(()) base_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    enum color GTY(()) color;
};

struct GTY(()) complex_struct {
    /* Nested anonymous struct (TYPE_STRUCT) */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } point;
    
    /* Bit fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque_ptr;
    
    /* Chain next pointer */
    struct complex_struct* GTY((chain_next)) next;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_data;
    void* GTY(()) user_handle;
};

/* TYPE_UNION: Union types */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
    struct base_struct* GTY(()) struct_ptr;
};

/* Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum GTY(()) {
        TAG_INT,
        TAG_DOUBLE,
        TAG_STRING,
        TAG_STRUCT
    } tag;
    
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY(()) s;
        struct base_struct* GTY(()) p;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* GTY(()) base_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(int, char*);
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef base_ptr* GTY(()) ptr_to_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[3][3];
typedef char* GTY(()) string_array[8];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) dynamic_string;
typedef const char* GTY(()) const_string;

/* TYPE_CALLBACK: Function pointer type */
typedef int (*GTY(()) callback_func)(int, void*);

/* Struct containing callback */
struct GTY(()) callback_container {
    callback_func GTY(()) handler;
    void* GTY(()) user_data;
    int GTY(()) (*inline_callback)(int, char*);
};

/* Complex type with all dependencies */
struct GTY(()) master_type {
    /* TYPE_STRUCT nested */
    struct base_struct GTY(()) base;
    
    /* TYPE_UNION */
    union data_union GTY(()) data;
    
    /* TYPE_POINTER */
    struct master_type* GTY((chain_next)) next;
    struct master_type* GTY((chain_prev)) prev;
    
    /* TYPE_ARRAY */
    int GTY(()) scores[5];
    struct base_struct* GTY(()) ptr_array[3];
    
    /* TYPE_SCALAR */
    enum color GTY(()) primary_color;
    double GTY(()) weight;
    
    /* TYPE_STRING */
    char* GTY((length("strlen(%h)"))) description;
    const char* GTY(()) const_name;
    
    /* TYPE_CALLBACK */
    callback_func GTY(()) notify;
    
    /* TYPE_USER_STRUCT */
    struct user_struct* GTY(()) user_info;
    
    /* For variable-length array */
    int GTY(()) count;
    int GTY((length("%0.count"))) variable_array[1];
};

/* Variable-length array structure */
struct GTY(()) var_len_struct {
    int GTY(()) length;
    char GTY((length("%0.length"))) data[1];
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int GTY(()) value;
    struct tree_node* GTY(()) left;
    struct tree_node* GTY(()) right;
    struct tree_node* GTY(()) parent;
};

#endif /* TEST_TYPES_H */
