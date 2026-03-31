/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"  /* For GTY macro definitions */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
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

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    float GTY(()) value;
    enum color GTY(()) color;
    
    /* Anonymous struct inside */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned int GTY(()) flags : 4;
    unsigned int GTY(()) status : 2;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int GTY(()) int_val;
    float GTY(()) float_val;
    char GTY(()) char_val;
    void* GTY(()) ptr_val;
    
    /* Anonymous union variant */
    struct GTY(()) {
        long GTY(()) high;
        long GTY(()) low;
    } long_pair;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int GTY(()) user_id;
    char* GTY((length("strlen(%h.name) + 1"))) name;
};

/* TYPE_ARRAY: Array types */
typedef struct basic_struct GTY(()) struct_array[10];
typedef int GTY(()) int_matrix[5][5];
typedef union data_union GTY(()) union_array[20];

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(int, char*);
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct_ptr GTY(()) ptr_to_ptr;

/* TYPE_STRING: String types */
typedef char* GTY((string)) string_ptr;
typedef const char* GTY((string)) const_string_ptr;

/* TYPE_CALLBACK: Callback function pointer type */
typedef int (*GTY(()) callback_func)(const char*, void*);

/* Complex nested struct with all type kinds */
struct GTY(()) complex_nested {
    /* TYPE_STRUCT member */
    struct basic_struct GTY(()) base;
    
    /* TYPE_UNION member */
    union data_union GTY(()) data;
    
    /* TYPE_ARRAY members */
    int GTY(()) numbers[100];
    struct basic_struct GTY(()) structs[5];
    
    /* TYPE_POINTER members */
    struct complex_nested* GTY(()) next;
    struct complex_nested* GTY(()) prev;
    callback_func GTY(()) handler;
    
    /* TYPE_STRING members */
    char* GTY((string)) title;
    const char* GTY((string)) description;
    
    /* TYPE_SCALAR members */
    enum color GTY(()) theme;
    long GTY(()) timestamp;
    double GTY(()) precision;
    
    /* Chain pointers for GTY options */
    struct complex_nested* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chain_link;
    
    /* Length field for array */
    int GTY(()) count;
    struct basic_struct* GTY((length("%h.count"))) var_array;
    
    /* For desc/tag option */
    int GTY(()) type_tag;
    union {
        int GTY(()) int_data;
        float GTY(()) float_data;
        char* GTY((string)) str_data;
    } GTY((desc("%0.type_tag"))) tagged_data;
};

/* Linked list structure using chain_next */
struct GTY(()) linked_node {
    int GTY(()) data;
    struct linked_node* GTY((chain_next("%h.next"))) next;
    struct linked_node* GTY((chain_prev("%h.prev"))) prev;
};

/* Structure with callback */
struct GTY(()) callback_container {
    char* GTY((string)) name;
    callback_func GTY(()) callback;
    void* GTY(()) user_data;
};

#endif /* TEST_TYPES_H */
