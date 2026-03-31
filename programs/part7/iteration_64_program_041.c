/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_forward_decl;

/* Enum type for scalar coverage */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_scalar_struct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    bool bool_field;        /* TYPE_SCALAR (C++) */
    color_t enum_field;     /* TYPE_SCALAR */
};

/* Structure with array fields - TYPE_ARRAY */
struct GTY(()) array_struct {
    int fixed_array[10];           /* Fixed-size array */
    char* GTY((length("strlen(%h.str_field) + 1"))) str_field;  /* TYPE_STRING */
    struct basic_scalar_struct* GTY((skip)) skip_ptr;  /* Pointer to ignore */
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_struct {
    struct basic_scalar_struct* scalar_ptr;      /* Pointer to struct */
    struct array_struct** double_ptr;            /* Pointer to pointer */
    struct opaque_forward_decl* opaque_ptr;      /* Pointer to undefined type */
    void* void_ptr;                              /* void pointer */
};

/* Linked list structure for recursive traversal */
struct GTY(()) linked_list {
    int data;
    struct linked_list* GTY((tag("0"))) next;
};

/* Global variables to ensure processing */
extern struct basic_scalar_struct GTY(()) global_scalar_struct;
extern struct array_struct GTY(()) global_array_struct[5];
extern struct pointer_struct* GTY(()) global_pointer_struct;

#endif /* TEST_BASIC_STRUCTS_H */
