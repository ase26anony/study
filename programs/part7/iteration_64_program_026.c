/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* Enum type for TYPE_SCALAR */
typedef enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_node {
    int id;                     /* TYPE_SCALAR - int */
    char tag;                   /* TYPE_SCALAR - char */
    long size;                  /* TYPE_SCALAR - long */
    double weight;              /* TYPE_SCALAR - double */
    color_t color;              /* TYPE_SCALAR - enum */
    bool visited;               /* TYPE_SCALAR - bool (C++) */
};

/* Structure with array fields - TYPE_ARRAY */
struct GTY(()) array_container {
    int counts[10];             /* TYPE_ARRAY - fixed size */
    struct basic_node *GTY((length("len"))) items; /* TYPE_ARRAY - variable length */
    unsigned int len;
    char name[32];              /* TYPE_ARRAY - char array */
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_node {
    struct basic_node *GTY((tag("0"))) next;    /* TYPE_POINTER */
    struct array_container *GTY((skip)) skipme; /* TYPE_POINTER with skip */
    opaque_ptr_t opaque;        /* TYPE_POINTER to undefined type */
    void *generic_ptr;          /* TYPE_POINTER to void */
};

/* Linked list structure for recursive traversal */
struct GTY(()) linked_list {
    int value;
    struct linked_list *GTY((tag("0"))) next;
};

/* Global variables to ensure processing */
extern struct basic_node GTY((root)) *global_basic_list;
extern struct array_container GTY((root)) global_container;
extern struct pointer_node *GTY((root)) global_pointer_chain;

#endif /* TEST_BASIC_STRUCTS_H */
