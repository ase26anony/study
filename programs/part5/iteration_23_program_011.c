/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY header */
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

/* TYPE_STRING: String type */
typedef char* GTY((length("strlen(%h)"))) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int int_val;
    double double_val;
    char* GTY((tag("0"))) str_val;
};

/* Tagged union with desc option */
union GTY((desc("%1.type"), tag("type"))) tagged_union {
    int type;
    struct {
        int x;
        int y;
    } GTY((tag("1"))) point;
    struct {
        float radius;
        color_t color;
    } GTY((tag("2"))) circle;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) simple_struct* GTY(()) ptr_array[5];

/* TYPE_STRUCT: Basic struct */
struct GTY(()) simple_struct {
    int id;
    char name[32];
    color_t color;
};

/* Struct with nested anonymous struct */
struct GTY(()) nested_struct {
    int outer_val;
    struct {
        int inner_a;
        int inner_b;
    } GTY(()) inner;
    union {
        int as_int;
        float as_float;
    } GTY(()) data;
};

/* Struct with bit-fields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_field;
};

/* Struct with chain_next/chain_prev for linked list */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
    int data;
    struct linked_node* GTY(()) next;
    struct linked_node* GTY(()) prev;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_data;
    void* user_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_container {
    /* Simple pointers */
    int* GTY(()) int_ptr;
    struct simple_struct* GTY(()) struct_ptr;
    union basic_union* GTY(()) union_ptr;
    
    /* Pointer to pointer */
    int** GTY(()) int_ptr_ptr;
    
    /* Void pointer */
    void* GTY(()) void_ptr;
    
    /* Function pointer */
    callback_func GTY(()) callback;
    
    /* Pointer to array */
    int (*GTY(()) array_ptr)[10];
    
    /* Pointer to undefined type */
    struct opaque_struct* GTY(()) opaque_ptr;
};

/* Multi-dimensional array */
struct GTY(()) matrix_container {
    int matrix[3][3];
    double cube[2][2][2];
};

/* Complex struct with all type kinds */
struct GTY(()) master_struct {
    /* Scalar */
    scalar_int count;
    scalar_double value;
    color_t color;
    
    /* String */
    string_type GTY(()) name;
    
    /* Struct */
    struct simple_struct GTY(()) data;
    
    /* Union */
    union tagged_union GTY(()) variant;
    
    /* Array */
    int_array GTY(()) numbers;
    
    /* Pointer */
    struct pointer_container* GTY(()) ptrs;
    
    /* Callback */
    callback_func GTY(()) handler;
    
    /* Nested anonymous struct with bitfields */
    struct {
        unsigned int : 4;
        unsigned int mode : 2;
        unsigned int : 2;
    } GTY(()) flags;
    
    /* Chain for linked structure */
    struct linked_node* GTY(()) head;
};

/* Global variable declarations */
extern struct master_struct GTY(()) global_master;
extern struct pointer_container GTY(()) global_pointers;
extern union tagged_union GTY(()) global_union;

#endif /* TEST_TYPES_H */
