/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_forward_decl;

/* Enum type for TYPE_SCALAR */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;              /* TYPE_SCALAR */
    long long_field;            /* TYPE_SCALAR */
    char char_field;            /* TYPE_SCALAR */
    float float_field;          /* TYPE_SCALAR */
    double double_field;        /* TYPE_SCALAR */
    bool bool_field;            /* TYPE_SCALAR (C++) */
    color_t enum_field;         /* TYPE_SCALAR */
    
    /* TYPE_ARRAY */
    int int_array[10];
    char char_array[20];
    
    /* TYPE_POINTER */
    struct basic_struct *GTY((skip)) next;  /* Skip pointer */
    struct opaque_forward_decl *GTY((tag("0"))) opaque_ptr;  /* TYPE_UNDEFINED */
    
    /* TYPE_STRING */
    const char *GTY((length("strlen(%h.string_field) + 1"))) string_field;
    char *GTY((length("strlen(%h.dyn_string) + 1"))) dyn_string;
};

/* Array of structures */
typedef struct GTY(()) basic_struct basic_struct_array[5];

/* Global variable with GTY markup */
extern struct GTY(()) basic_struct *global_struct_ptr;

/* Linked list structure for recursive traversal */
struct GTY(()) linked_list {
    int data;
    struct linked_list *GTY((skip)) next;
    struct linked_list *GTY((skip)) prev;
};

#endif /* TEST_BASIC_STRUCTS_H */
