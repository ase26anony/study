/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY headers */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum GTY(()) color_enum {
    RED,
    GREEN,
    BLUE
} color_enum;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;

/* TYPE_UNION: Various union types */
union GTY(()) basic_union {
    int i;
    double d;
    char* s;
};

union GTY(()) tagged_union {
    int tag;
    struct {
        int x;
        int y;
    } GTY((desc("%0.tag"))) point;
    struct {
        float radius;
        color_enum color;
    } GTY((desc("%0.tag"))) circle;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef union basic_union GTY(()) union_array[5][5];

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen(%0)+1"))) counted_string;
typedef const char* GTY(()) constant_string;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*GTY(()) callback_func)(int, double);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* TYPE_STRUCT: Basic struct types */
struct GTY(()) simple_struct {
    int id;
    double value;
    char name[32];
};

struct GTY(()) nested_struct {
    struct simple_struct GTY((tag("0"))) base;
    union tagged_union GTY((tag("1"))) data;
    int_array GTY(()) numbers;
};

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int : 24; /* padding */
    color_enum color : 2;
};

/* Anonymous struct within struct */
struct GTY(()) container_struct {
    struct {
        int x;
        int y;
    } GTY(()) point;
    struct {
        int width;
        int height;
    } GTY(()) size;
    union {
        int i;
        float f;
    } GTY(()) data;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_defined_struct {
    int user_id;
    char* GTY((skip)) user_data; /* Skip in GC */
    struct opaque_struct* GTY(()) opaque_ref;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
    /* Simple pointers */
    struct simple_struct* GTY(()) simple_ptr;
    union basic_union* GTY(()) union_ptr;
    
    /* Pointer to pointer */
    struct nested_struct** GTY(()) nested_pptr;
    
    /* Void pointer */
    void* GTY(()) void_ptr;
    
    /* Function pointer */
    callback_func GTY(()) callback;
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
    
    /* Self-referential pointer */
    struct pointer_struct* GTY((chain_next("%0.next"))) next;
    struct pointer_struct* GTY((chain_prev("%0.prev"))) prev;
};

/* Complex struct with all type kinds */
struct GTY(()) master_struct {
    /* SCALAR */
    scalar_int id;
    color_enum color;
    
    /* STRUCT */
    struct nested_struct GTY(()) nested;
    
    /* UNION */
    union tagged_union GTY((desc("%0.union_tag"))) data_union;
    int union_tag;
    
    /* ARRAY */
    struct simple_struct GTY(()) struct_array[5];
    counted_string GTY(()) string_array[3];
    
    /* POINTER */
    struct pointer_struct* GTY(()) ptr_struct;
    compare_func GTY(()) comparator;
    
    /* STRING */
    counted_string GTY(()) dynamic_string;
    constant_string GTY(()) static_string;
    
    /* CALLBACK */
    callback_func GTY(()) handler;
    
    /* Reference to undefined type */
    struct opaque_struct* GTY(()) opaque;
    
    /* Bitfield */
    struct bitfield_struct GTY(()) flags;
    
    /* Anonymous union */
    union {
        int counter;
        double precision;
    } GTY(()) metrics;
};

/* Linked list structure for chain testing */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((chain_next("%h.next"))) next;
    struct list_node* GTY((chain_prev("%h.prev"))) prev;
};

/* Variable length array structure */
struct GTY(()) varray_struct {
    int length;
    int GTY((length("%0.length"))) items[];
};

#endif /* TEST_TYPES_H */
