/* basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef BASIC_STRUCTS_H
#define BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRUCT with scalar fields */
struct GTY(()) scalar_container {
    int int_field;          /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    bool bool_field;        /* TYPE_SCALAR (C++) */
    color_t enum_field;     /* TYPE_SCALAR */
};

/* TYPE_ARRAY examples */
struct GTY(()) array_container {
    int fixed_array[10];           /* TYPE_ARRAY fixed size */
    char* GTY((length("str_len"))) variable_array; /* TYPE_ARRAY variable length */
    int str_len;
    
    /* Nested array in struct */
    struct GTY(()) nested {
        double matrix[3][3];       /* TYPE_ARRAY multi-dimensional */
    } nested_array;
};

/* TYPE_POINTER chains */
struct GTY(()) linked_node {
    int value;
    struct linked_node* GTY((skip)) next;  /* TYPE_POINTER with skip */
    struct linked_node* GTY((tag("0"))) prev; /* TYPE_POINTER */
};

/* Global variables to ensure processing */
extern GTY(()) struct scalar_container global_scalar;
extern GTY(()) struct array_container global_array;
extern GTY(()) struct linked_node* GTY((chain_next("%h.next"))) global_list;

#endif /* BASIC_STRUCTS_H */
