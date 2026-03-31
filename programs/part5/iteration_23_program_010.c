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
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($) + 1"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int ival;
    float fval;
    char* GTY((length("strlen($) + 1"))) sval;
};

union GTY(()) tagged_union {
    int tag;
    struct {
        int x;
        int y;
    } GTY(()) point;
    struct {
        float radius;
        color_t color;
    } GTY(()) circle;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) simple_struct* GTY(()) ptr_array[5];

/* TYPE_POINTER: Various pointer types */
typedef void* GTY(()) void_ptr;
typedef struct GTY(()) simple_struct* GTY(()) struct_ptr;
typedef union GTY(()) basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef int** GTY(()) int_ptr_ptr;
typedef callback_func GTY(()) callback_ptr;

/* TYPE_STRUCT: Struct types with various features */
struct GTY(()) simple_struct {
    int id;
    char name[32];
    color_t color;
    struct simple_struct* GTY((chain_next("%h.next"))) next;
};

struct GTY(()) complex_struct {
    /* Nested anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } position;
    
    /* Anonymous union */
    union GTY(()) {
        int int_val;
        float float_val;
    } value;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    
    /* Pointer member */
    struct complex_struct* GTY(()) self_ptr;
    
    /* Array member */
    int GTY(()) scores[5];
    
    /* String member */
    char* GTY((length("strlen($) + 1"))) description;
    
    /* Union member */
    union tagged_union GTY(()) data;
    
    /* Callback member */
    callback_func GTY(()) handler;
    
    /* Chain pointers for linked list */
    struct complex_struct* GTY((chain_next("%h.next"))) next;
    struct complex_struct* GTY((chain_prev("%h.prev"))) prev;
};

/* Multi-dimensional array in struct */
struct GTY(()) matrix_struct {
    int GTY(()) data[3][3];
    float GTY(()) weights[2][4][2];
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int custom_id;
    void* GTY((skip)) user_data;  /* Skip this field for GC */
    struct user_struct* GTY(()) next;
};

#endif /* TEST_TYPES_H */
