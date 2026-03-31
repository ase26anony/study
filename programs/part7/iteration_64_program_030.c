/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_undefined;
typedef struct opaque_undefined *opaque_ptr_t GTY((user));

/* Enum type for scalar coverage */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct basic_scalars GTY(()) {
    int int_field;
    long long_field;
    char char_field;
    float float_field;
    double double_field;
    bool bool_field;
    color_t enum_field;
};

/* Structure with array fields - TYPE_ARRAY */
struct with_arrays GTY(()) {
    int int_array[10];
    char char_array[256];
    struct basic_scalars *ptr_array[5] GTY((length("5")));
    double multi_dim[3][4];
};

/* Structure with pointer fields - TYPE_POINTER */
struct with_pointers GTY(()) {
    struct basic_scalars *next GTY((tag("0")));
    struct with_arrays *array_ptr;
    void *generic_ptr;
    const char *const_string;
    struct opaque_undefined *undefined_ptr;  /* TYPE_UNDEFINED */
};

/* Linked list for recursive traversal */
struct linked_list GTY(()) {
    int data;
    struct linked_list *next GTY((tag("0")));
    struct linked_list *prev GTY((tag("0")));
};

/* Global variables to ensure processing */
extern struct basic_scalars global_scalar_struct GTY((user));
extern struct with_arrays global_array_struct;
extern struct linked_list *global_list GTY((tag("0")));

#endif /* TEST_BASIC_STRUCTS_H */
