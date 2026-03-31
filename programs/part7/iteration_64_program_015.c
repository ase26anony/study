/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;
typedef struct opaque_type *opaque_ptr_t GTY(());

/* Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRUCT with scalar fields */
struct basic_struct GTY(()) {
    int int_field;
    long long_field;
    char char_field;
    double double_field;
    float float_field;
    bool bool_field;
    color_t enum_field;
};

/* TYPE_ARRAY within structure */
struct array_container GTY(()) {
    int fixed_array[10];
    struct basic_struct struct_array[5];
    char* string_array[3] GTY((length("3")));
};

/* TYPE_POINTER fields */
struct pointer_struct GTY(()) {
    struct basic_struct *next GTY((skip));
    struct array_container *container;
    void *generic_ptr;
    opaque_ptr_t opaque_ptr;  /* TYPE_UNDEFINED pointer */
};

/* Chain of structures for recursive traversal */
struct linked_node GTY(()) {
    int data;
    struct linked_node *next GTY((skip));
    struct linked_node *prev;
};

/* Global variables to ensure processing */
extern struct basic_struct global_struct GTY(());
extern struct array_container global_array_container GTY(());
extern struct linked_node *global_list GTY(());

#endif /* TEST_BASIC_STRUCTS_H */
