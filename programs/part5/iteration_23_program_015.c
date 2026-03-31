/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro definitions */

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
    
    /* Bit-fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
    
    /* Regular members */
    struct base_struct GTY(()) *base;  /* TYPE_POINTER to TYPE_STRUCT */
    int GTY(()) count;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_data;
    void GTY((skip)) *skip_ptr;  /* Skip this in marking */
};

/* TYPE_UNION: Regular union */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char GTY(()) char_val;
    struct base_struct GTY(()) *struct_ptr;  /* TYPE_POINTER */
};

/* TYPE_UNION: Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum GTY(()) union_type {
        INT_TYPE,
        DOUBLE_TYPE,
        STRUCT_TYPE
    } type;
    
    /* Anonymous union */
    union GTY(()) {
        int GTY(()) i;
        double GTY(()) d;
        struct base_struct GTY(()) *s;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct GTY(()) *base_ptr_t;
typedef union data_union GTY(()) *union_ptr_t;
typedef int GTY(()) *int_ptr_t;
typedef void GTY(()) *void_ptr_t;

/* Function pointer type for TYPE_CALLBACK */
typedef void GTY(()) (*callback_func)(int, const char*);

/* TYPE_CALLBACK: Struct containing callback */
struct GTY(()) callback_container {
    callback_func GTY(()) handler;
    int GTY(()) context;
};

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[8];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix[3][3];
typedef struct base_struct GTY(()) struct_matrix[2][2];

/* TYPE_STRING: String types */
typedef const char GTY(()) *string_ptr;
typedef char GTY(()) *mutable_string_ptr;

/* Chain structures for linked list testing */
struct GTY((chain_next ("next"))) chain_node {
    int GTY(()) value;
    struct chain_node GTY(()) *next;
    struct chain_node GTY(()) *prev;
};

/* Length-tagged array structure */
struct GTY(()) variable_array {
    int GTY(()) length;
    int GTY((length ("%0.length"))) data[];
};

/* Nested complex type with all kinds */
struct GTY(()) master_container {
    /* TYPE_STRUCT */
    struct base_struct GTY(()) base;
    
    /* TYPE_UNION */
    union data_union GTY(()) data;
    
    /* TYPE_POINTER to various types */
    struct base_struct GTY(()) *struct_ptr;
    union data_union GTY(()) *union_ptr;
    int GTY(()) *int_ptr;
    void GTY(()) *void_ptr;
    
    /* TYPE_ARRAY */
    int GTY(()) int_arr[5];
    struct base_struct GTY(()) struct_arr[3];
    
    /* TYPE_STRING */
    const char GTY(()) *string_field;
    char GTY(()) *mutable_string;
    
    /* TYPE_CALLBACK */
    callback_func GTY(()) callback;
    
    /* TYPE_SCALAR */
    enum color GTY(()) color;
    int GTY(()) scalar;
    double GTY(()) double_scalar;
    
    /* Chain pointer */
    struct chain_node GTY(()) *chain_head;
    
    /* Variable length array */
    struct variable_array GTY(()) *var_array;
    
    /* User struct */
    struct user_struct GTY(()) user;
    
    /* Opaque pointer (TYPE_UNDEFINED) */
    struct opaque_struct GTY(()) *opaque;
};

/* Global variable declarations */
extern struct master_container GTY(()) global_container;
extern struct chain_node GTY(()) *global_chain;
extern string_ptr GTY(()) global_strings[];
extern int GTY(()) global_int_matrix[4][4];

#endif /* TEST_TYPES_H */
