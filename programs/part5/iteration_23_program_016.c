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

/* TYPE_STRING: String type */
typedef char* GTY((length("strlen($) + 1"))) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, double);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    double d;
    char* GTY((tag("0"))) str;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_DOUBLE, TAG_STRING } GTY(()) tag;
    struct {
        int ival;
    } GTY((desc("TAG_INT"))) as_int;
    struct {
        double dval;
    } GTY((desc("TAG_DOUBLE"))) as_double;
    struct {
        char* GTY((length("strlen($) + 1"))) sval;
    } GTY((desc("TAG_STRING"))) as_string;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct GTY(()) simple_struct* GTY(()) ptr_array[5];

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    int id;
    char name[50];
    double value;
};

struct GTY(()) complex_struct {
    /* Nested anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    /* Bit fields */
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    
    /* Nested union */
    union GTY(()) {
        int ival;
        float fval;
    } data;
    
    /* Pointer member */
    struct complex_struct* GTY((chain_next("$->next"))) next;
    
    /* Array member */
    int GTY(()) scores[5];
    
    /* String member */
    char* GTY((length("strlen($) + 1"))) description;
    
    /* Callback member */
    callback_func GTY(()) handler;
    
    /* Scalar enum member */
    color_t GTY(()) color;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int custom_field;
    void* GTY((skip)) opaque_data;
};

/* TYPE_POINTER: Various pointer types */
typedef void* GTY(()) void_ptr;
typedef struct simple_struct* GTY(()) simple_ptr;
typedef struct complex_struct** GTY(()) double_ptr;
typedef int (*GTY(()) func_ptr)(int, int);
typedef callback_func* GTY(()) callback_ptr;

/* Container struct that references all types */
struct GTY(()) master_container {
    /* TYPE_STRUCT reference */
    struct simple_struct GTY(()) simple;
    
    /* TYPE_USER_STRUCT reference */
    struct user_defined_struct GTY(()) user_struct;
    
    /* TYPE_UNION reference */
    union basic_union GTY(()) basic_union;
    union tagged_union GTY(()) tagged_union;
    
    /* TYPE_POINTER references */
    struct opaque_struct* GTY(()) opaque_ptr;
    void_ptr GTY(()) void_pointer;
    simple_ptr GTY(()) simple_pointer;
    double_ptr GTY(()) double_pointer;
    func_ptr GTY(()) function_pointer;
    callback_ptr GTY(()) callback_pointer;
    
    /* TYPE_ARRAY references */
    int_array GTY(()) int_array_field;
    ptr_array GTY(()) ptr_array_field;
    
    /* TYPE_SCALAR references */
    scalar_int GTY(()) int_field;
    scalar_double GTY(()) double_field;
    color_t GTY(()) enum_field;
    
    /* TYPE_STRING reference */
    string_type GTY(()) string_field;
    
    /* TYPE_CALLBACK reference */
    callback_func GTY(()) callback_field;
    
    /* Self-referential pointer for linked list */
    struct master_container* GTY((chain_next("$->next_container"))) next_container;
};

/* Global variable declarations */
extern struct master_container GTY(()) global_container;
extern struct simple_struct GTY(()) global_simple_array[3];
extern union basic_union GTY(()) global_union_array[2];

#endif /* TEST_TYPES_H */
