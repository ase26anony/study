/* test_types.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;

/* TYPE_STRING: String types */
typedef char* GTY((length("strlen($1) + 1"))) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    float f;
    char* GTY((tag("0"))) str;
};

union GTY(()) tagged_union {
    int tag;
    struct {
        int x;
        int y;
    } GTY((tag("1"))) point;
    struct {
        float radius;
        color_t color;
    } GTY((tag("2"))) circle;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) point2d {
    int x;
    int y;
};

struct GTY(()) complex_struct {
    /* Nested anonymous struct */
    struct GTY(()) {
        int id;
        char name[32];
    } header;
    
    /* Bit fields */
    unsigned int flags : 4;
    unsigned int mode : 3;
    
    /* Anonymous union within struct */
    union GTY(()) {
        int count;
        float percentage;
    };
    
    /* Regular members */
    color_t color;
    struct point2d* GTY((skip)) next_point;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_defined_struct {
    /* User struct with custom handling */
    void* GTY((skip)) user_data;
    int user_id;
    const char* user_name;
};

/* TYPE_ARRAY: Array types */
typedef struct point2d GTY(()) point_array[10];
typedef int GTY(()) int_matrix[3][3];
typedef union basic_union GTY(()) union_array[5];

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    /* Simple pointers */
    struct point2d* GTY(()) struct_ptr;
    union tagged_union* GTY(()) union_ptr;
    
    /* Pointer to pointer */
    struct complex_struct** GTY(()) ptr_to_ptr;
    
    /* Void pointer */
    void* GTY((skip)) void_ptr;
    
    /* Function pointer */
    callback_func GTY(()) callback;
    
    /* String pointer (also TYPE_STRING) */
    string_ptr GTY(()) dynamic_string;
    
    /* Array of pointers */
    color_t* GTY(()) color_array[4];
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
    
    /* Pointer to opaque (undefined) type */
    struct opaque_struct* GTY(()) opaque_ptr;
};

/* Chainable struct for linked list testing */
struct GTY((chain_next("%h.next"))) linked_node {
    int data;
    struct linked_node* GTY((skip)) next;
    struct linked_node* GTY((skip)) prev;
};

/* Struct with variable-length array */
struct GTY(()) var_len_struct {
    int count;
    int GTY((length("%h.count"))) data[];
};

/* Extern declarations for global variables */
extern struct complex_struct GTY(()) global_complex;
extern union tagged_union GTY(()) global_union;
extern point_array GTY(()) global_points;
extern struct pointer_container GTY(()) global_pointers;

#endif /* TEST_TYPES_H */
