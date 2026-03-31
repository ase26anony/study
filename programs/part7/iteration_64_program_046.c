/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color GTY(());

/* TYPE_STRUCT: Basic structure with multiple scalar fields */
struct basic_struct GTY(()) {
    int int_field;          /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    bool bool_field;        /* TYPE_SCALAR (C++) */
    color enum_field;       /* TYPE_SCALAR (enum) */
};

/* TYPE_ARRAY: Arrays within structures */
struct array_struct GTY(()) {
    int fixed_array[10];                    /* TYPE_ARRAY fixed size */
    struct basic_struct struct_array[5];    /* TYPE_ARRAY of structs */
    char* GTY((length("strlen(%h.str_field) + 1"))) str_field; /* TYPE_STRING */
};

/* TYPE_POINTER: Pointer fields */
struct pointer_struct GTY(()) {
    struct basic_struct* direct_ptr;        /* TYPE_POINTER to struct */
    struct basic_struct** double_ptr;       /* TYPE_POINTER to pointer */
    void* opaque_ptr;                       /* TYPE_POINTER to incomplete */
};

/* Chain of structures for recursive traversal */
struct linked_node GTY(()) {
    int data;
    struct linked_node* GTY((skip)) next;   /* TYPE_POINTER with skip */
    struct linked_node* GTY((chain_next("%h.next"))) chain_next;
};

/* Global variables to ensure processing */
extern struct basic_struct global_struct GTY(());
extern struct array_struct global_array GTY(());
extern struct pointer_struct* global_pointer GTY(());

#endif /* TEST_BASIC_STRUCTS_H */
