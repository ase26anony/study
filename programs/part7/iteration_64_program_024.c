/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* Enum type for scalar coverage */
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
    int fixed_array[10];               /* TYPE_ARRAY */
    char string_array[5][20];          /* TYPE_ARRAY of TYPE_ARRAY */
    struct basic_struct struct_array[3]; /* TYPE_ARRAY of TYPE_STRUCT */
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_struct {
    int *int_ptr;                      /* TYPE_POINTER to TYPE_SCALAR */
    struct basic_struct *struct_ptr;   /* TYPE_POINTER to TYPE_STRUCT */
    struct opaque_type *opaque_ptr;    /* TYPE_POINTER to TYPE_UNDEFINED */
    void *void_ptr;                    /* TYPE_POINTER with TYPE_UNDEFINED */
    char *string_ptr;                  /* TYPE_POINTER to TYPE_STRING */
};

/* Linked list for recursive traversal */
struct GTY(()) linked_node {
    int data;
    struct linked_node * GTY((skip)) next_skip;  /* GTY((skip)) pointer */
    struct linked_node * GTY((chain_next ("%h.next"))) next;
};

/* Global variables to ensure processing */
extern GTY(()) struct basic_struct global_basic;
extern GTY(()) struct array_struct global_array;
extern GTY(()) struct pointer_struct global_pointer;
extern GTY(()) struct linked_node *global_list;

#endif /* TEST_BASIC_STRUCTS_H */
