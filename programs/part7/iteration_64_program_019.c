/* test-basic-structs.h - Basic structures covering TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* Enum type for TYPE_SCALAR */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;           /* TYPE_SCALAR */
    long long_field;         /* TYPE_SCALAR */
    double double_field;     /* TYPE_SCALAR */
    float float_field;       /* TYPE_SCALAR */
    char char_field;         /* TYPE_SCALAR */
    bool bool_field;         /* TYPE_SCALAR (C++) */
    color_t enum_field;      /* TYPE_SCALAR */
    
    /* Fixed-size array - TYPE_ARRAY */
    int array_field[10];     /* TYPE_ARRAY */
    
    /* Pointer to scalar - TYPE_POINTER */
    int* int_ptr;            /* TYPE_POINTER */
    
    /* String - TYPE_STRING */
    const char* GTY((skip)) name;  /* TYPE_STRING with skip */
    char* GTY((length("strlen(%h.name) + 1"))) dynamic_string; /* TYPE_STRING with length */
};

/* Array of structures - TYPE_ARRAY within TYPE_STRUCT */
struct GTY(()) array_container {
    struct basic_struct GTY((tag("0"))) items[5];  /* TYPE_ARRAY */
    int count;
};

/* Global variable with GTY markup */
extern struct basic_struct GTY(()) global_struct;

#endif /* TEST_BASIC_STRUCTS_H */
