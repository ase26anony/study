/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY headers */
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

/* TYPE_STRUCT: Multiple struct types with complex members */
struct GTY(()) base_struct {
    int GTY(()) id;
    char GTY(()) name[32];
    enum color GTY(()) color;
};

/* Struct with nested anonymous struct (TYPE_STRUCT) */
struct GTY(()) complex_struct {
    struct base_struct GTY(()) base;
    
    /* Anonymous struct */
    struct GTY(()) {
        int GTY(()) x;
        int GTY(()) y;
    } position;
    
    /* Bit-fields */
    unsigned GTY(()) flags : 4;
    unsigned GTY(()) status : 2;
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int GTY(()) user_id;
    void* GTY(()) user_data;
};

/* TYPE_UNION: Various union types */
union GTY(()) data_union {
    int GTY(()) int_val;
    double GTY(()) double_val;
    char* GTY(()) string_val;
    struct base_struct GTY(()) struct_val;
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
        struct base_struct GTY(()) st;
    } data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct* GTY(()) base_ptr;
typedef union data_union* GTY(()) union_ptr;
typedef void (*GTY(()) callback_func)(int, const char*);

/* Function pointer type for TYPE_CALLBACK */
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct base_struct GTY(()) struct_array[5];
typedef union data_union GTY(()) union_array[8];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix[3][3];
typedef struct base_struct* GTY(()) ptr_array[4][2];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%h)"))) dynamic_string;
typedef const char* GTY(()) const_string;

/* Master struct containing references to all types */
struct GTY(()) master_container {
    /* TYPE_STRUCT */
    struct complex_struct GTY(()) complex;
    
    /* TYPE_USER_STRUCT */
    struct user_struct GTY(()) user;
    
    /* TYPE_UNION */
    union data_union GTY(()) data;
    
    /* TYPE_POINTER */
    struct base_struct* GTY(()) base_ptr;
    void* GTY(()) void_ptr;
    compare_func GTY(()) comparator;
    
    /* TYPE_ARRAY */
    int GTY(()) numbers[5];
    struct base_struct GTY(()) objects[3];
    matrix GTY(()) transformation;
    
    /* TYPE_SCALAR */
    scalar_int GTY(()) count;
    scalar_double GTY(()) value;
    color_t GTY(()) color;
    
    /* TYPE_STRING */
    dynamic_string GTY(()) name;
    const_string GTY(()) constant;
    
    /* Chain pointers for linked list */
    struct master_container* GTY((chain_next("%h.next"))) next;
    struct master_container* GTY((chain_prev("%h.prev"))) prev;
    
    /* Array with length field */
    int* GTY((length("%h.dyn_length"))) dynamic_array;
    int GTY(()) dyn_length;
};

/* TYPE_CALLBACK: Struct with callback function pointer */
struct GTY(()) callback_container {
    compare_func GTY(()) sorter;
    callback_func GTY(()) handler;
    void* GTY(()) context;
};

/* Global variable declarations */
extern struct master_container GTY(()) global_master;
extern struct callback_container GTY(()) global_callbacks;
extern union data_union GTY(()) global_union_array[];

#endif /* TEST_TYPES_H */
