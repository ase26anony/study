/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRUCT with scalar fields */
struct GTY(()) basic_struct {
    int scalar_int;                    /* TYPE_SCALAR */
    long scalar_long;                  /* TYPE_SCALAR */
    double scalar_double;              /* TYPE_SCALAR */
    color_t scalar_enum;               /* TYPE_SCALAR */
    unsigned char scalar_char;         /* TYPE_SCALAR */
};

/* TYPE_ARRAY examples */
struct GTY(()) array_container {
    int fixed_array[10];               /* TYPE_ARRAY */
    struct basic_struct struct_array[5]; /* TYPE_ARRAY of TYPE_STRUCT */
};

/* TYPE_POINTER examples */
struct GTY(()) pointer_container {
    struct basic_struct *GTY((skip)) opaque_ptr;  /* TYPE_POINTER with skip */
    struct array_container *nested_ptr;           /* TYPE_POINTER to TYPE_STRUCT */
    void *void_ptr;                               /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* TYPE_STRING examples */
struct GTY(()) string_container {
    const char *GTY((tag("0"))) static_string;    /* TYPE_STRING */
    char *dynamic_string;                         /* TYPE_STRING */
};

/* Chain of structures for recursive traversal */
struct GTY(()) linked_node {
    int value;
    struct linked_node *GTY((skip)) next;         /* TYPE_POINTER in linked list */
};

/* Global variables to ensure processing */
extern GTY(()) struct basic_struct global_basic;
extern GTY(()) struct array_container global_array;
extern GTY(()) struct pointer_container global_pointers;

#endif /* TEST_BASIC_STRUCTS_H */
