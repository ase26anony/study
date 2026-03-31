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
    int fixed_array[10];               /* TYPE_ARRAY fixed size */
    char string_array[256];            /* TYPE_ARRAY as string buffer */
    struct scalar_container* GTY((length("len"))) var_array; /* TYPE_ARRAY variable length */
    int len;
};

/* TYPE_POINTER examples */
struct GTY(()) pointer_container {
    int* int_ptr;                      /* TYPE_POINTER to scalar */
    struct scalar_container* struct_ptr; /* TYPE_POINTER to struct */
    struct array_container** double_ptr; /* TYPE_POINTER to pointer */
    void* opaque_ptr;                  /* TYPE_POINTER to incomplete type */
};

/* TYPE_STRING examples */
struct GTY(()) string_container {
    const char* GTY((tag("0"))) const_string; /* TYPE_STRING */
    char* mutable_string;              /* TYPE_STRING */
};

/* Chain of structures for recursive traversal */
struct GTY(()) linked_node {
    int data;
    struct linked_node* GTY((skip)) next; /* TYPE_POINTER with skip */
    struct linked_node* GTY((chain_next("%h.next"))) chain_next;
};

/* Global variables to ensure processing */
extern GTY(()) struct scalar_container global_scalar;
extern GTY(()) struct array_container global_array;
extern GTY(()) struct pointer_container global_pointers;

#endif /* BASIC_STRUCTS_H */
