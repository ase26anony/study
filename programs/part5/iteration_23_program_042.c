/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;

/* TYPE_STRUCT: Regular struct with nested members */
struct GTY(()) base_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    struct base_struct* GTY((skip)) next;  /* Will be handled by pointer type */
};

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int GTY(()) flag1 : 1;
    unsigned int GTY(()) flag2 : 2;
    unsigned int GTY(()) value : 10;
    unsigned int GTY(()) : 3;  /* Padding */
};

/* Anonymous struct within a struct */
struct GTY(()) container_struct {
    struct {
        int GTY(()) x;
        int GTY(()) y;
    } GTY(()) point;
    int GTY(()) data;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int GTY(()) custom_field;
    void* GTY((skip)) user_data;
};

/* TYPE_UNION: Regular union */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
};

/* Tagged union within a struct */
struct GTY(()) tagged_union_container {
    enum { INT_TYPE, DOUBLE_TYPE, STRING_TYPE } GTY(()) tag;
    union {
        int GTY(()) i;
        double GTY(()) d;
        char* GTY(()) s;
    } GTY(()) value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* GTY(()) base_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(int, char*);  /* Function pointer */
typedef int** GTY(()) ptr_to_ptr;

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Callback function pointer type */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Struct using callback type */
struct GTY(()) callback_container {
    compare_func GTY(()) comparator;
    void* GTY(()) data;
    int GTY(()) count;
};

/* Complex struct with multiple type kinds */
struct GTY(()) master_struct {
    /* Scalar types */
    int GTY(()) id;
    color_t GTY(()) color;
    
    /* Pointer types */
    struct base_struct* GTY((chain_next("next"))) first;
    struct master_struct* GTY(()) self_ptr;
    
    /* Array type */
    int GTY(()) scores[5];
    
    /* Union type */
    union data_union GTY(()) data;
    
    /* String type */
    char* GTY((length("strlen(%h)"))) name;
    
    /* Nested struct */
    struct bitfield_struct GTY(()) flags;
    
    /* Callback type */
    compare_func GTY(()) sorter;
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque;
};

/* Chainable struct for testing chain_next/chain_prev */
struct GTY(()) chainable {
    int GTY(()) value;
    struct chainable* GTY((chain_next("next"), chain_prev("prev"))) next;
    struct chainable* GTY((chain_next("next"), chain_prev("prev"))) prev;
};

#endif /* TEST_TYPES_H */
