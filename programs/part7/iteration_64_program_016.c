/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;

/* Enum type for TYPE_SCALAR */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;          /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    bool bool_field;        /* TYPE_SCALAR (C++) */
    color_t enum_field;     /* TYPE_SCALAR */
};

/* Structure with array fields - TYPE_ARRAY */
struct GTY(()) array_struct {
    int fixed_array[10];            /* Fixed-size array */
    struct basic_struct *GTY((skip)) ptr_array[5]; /* Array of pointers */
    char string_array[3][20];       /* 2D array */
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_struct {
    struct basic_struct *GTY((tag("0"))) direct_ptr;    /* Direct pointer */
    struct opaque_type *GTY((skip)) opaque_ptr;         /* Pointer to undefined type */
    void *GTY((skip)) void_ptr;                         /* void pointer */
    int *GTY((skip)) int_ptr;                           /* Pointer to scalar */
};

/* Linked list structure for recursive traversal */
struct GTY(()) linked_node {
    int data;
    struct linked_node *GTY((tag("0"))) next;
};

/* Global variables to ensure processing */
extern struct basic_struct GTY((tag("0"))) global_basic;
extern struct array_struct GTY((tag("0"))) global_array;
extern struct pointer_struct GTY((tag("0"))) global_pointer;
extern struct linked_node *GTY((tag("0"))) global_list;

#endif /* TEST_BASIC_STRUCTS_H */
