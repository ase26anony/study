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

/* Basic structure with multiple scalar types - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;              /* TYPE_SCALAR */
    long long_field;            /* TYPE_SCALAR */
    char char_field;            /* TYPE_SCALAR */
    float float_field;          /* TYPE_SCALAR */
    double double_field;        /* TYPE_SCALAR */
    color_t enum_field;         /* TYPE_SCALAR (enum) */
    unsigned int bitfield:4;    /* TYPE_SCALAR (bitfield) */
    
    /* TYPE_ARRAY */
    int int_array[10];          /* Fixed-size array */
    char char_array[20];        /* Another fixed-size array */
    
    /* TYPE_POINTER */
    struct basic_struct *next;  /* Pointer to same type */
    struct opaque_type *opaque; /* Pointer to undefined type - TYPE_UNDEFINED */
    void *generic_ptr;          /* void pointer */
    
    /* TYPE_STRING */
    const char *name;           /* String pointer */
    char *dynamic_string;       /* Dynamic string */
};

/* Array of structures - TYPE_ARRAY of TYPE_STRUCT */
extern struct basic_struct GTY(()) global_struct_array[5];

/* Pointer to array - TYPE_POINTER to TYPE_ARRAY */
extern struct basic_struct * GTY(()) (*array_ptr)[5];

/* Chain of structures for recursive traversal */
struct GTY(()) linked_node {
    int value;
    struct linked_node * GTY((skip)) skip_ptr;  /* GTY((skip)) option */
    struct linked_node * GTY((null)) next;      /* GTY((null)) option */
};

/* Global variable with GTY markup */
extern struct basic_struct GTY(()) global_instance;

#endif /* TEST_BASIC_STRUCTS_H */
