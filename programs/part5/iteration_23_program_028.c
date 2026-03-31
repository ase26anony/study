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

/* TYPE_STRING: String type */
typedef char* GTY((length("strlen($1) + 1"))) string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int ival;
    float fval;
    char* GTY((tag("0"))) sval;
    void* GTY((tag("1"))) pval;
};

/* TYPE_STRUCT: Regular struct with various members */
struct GTY(()) complex_struct {
    /* Scalar members */
    int id;
    enum color color;
    float weight;
    
    /* Pointer member */
    struct complex_struct* GTY((skip)) next;
    
    /* String member */
    char* GTY((length("strlen($1) + 1"))) name;
    
    /* Array member */
    int GTY(()) scores[10];
    
    /* Union member */
    union basic_union data;
    
    /* Anonymous struct */
    struct GTY(()) {
        int x;
        int y;
    } position;
    
    /* Bit fields */
    unsigned int flags : 4;
    unsigned int status : 2;
    
    /* Callback member */
    callback_func handler;
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int custom_id;
    char* custom_name;
};

/* TYPE_ARRAY: Array types */
typedef struct complex_struct GTY(()) struct_array[5];
typedef union basic_union GTY(()) union_array[3][2];
typedef callback_func GTY(()) callback_array[4];

/* TYPE_POINTER: Various pointer types */
typedef struct complex_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef void (*GTY(()) func_ptr)(void);
typedef void* GTY(()) void_ptr;
typedef struct complex_struct** GTY(()) ptr_to_ptr;

/* Chain structure for linked list testing */
struct GTY(()) chain_node {
    int value;
    struct chain_node* GTY((chain_next("$1->next"))) next;
    struct chain_node* GTY((chain_prev("$1->prev"))) prev;
};

/* Structure with length field */
struct GTY(()) variable_array {
    int count;
    int GTY((length("%0.count"))) data[];
};

/* Nested structure with desc field */
struct GTY(()) outer_struct {
    int type;
    union GTY((desc("%0.type"))) {
        int ival;
        float fval;
        struct complex_struct* sptr;
    } data;
};

#endif /* TEST_TYPES_H */
