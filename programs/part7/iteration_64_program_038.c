/* Basic GTY-marked structures covering TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY */
#ifndef GTY_TEST_BASIC_H
#define GTY_TEST_BASIC_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Various scalar types */
typedef enum gty_color {
    GTY_RED,
    GTY_GREEN,
    GTY_BLUE
} gty_color;

/* TYPE_STRUCT: Basic structure with scalar fields */
struct GTY(()) gty_basic_struct {
    int field_int;              /* TYPE_SCALAR */
    long field_long;            /* TYPE_SCALAR */
    double field_double;        /* TYPE_SCALAR */
    gty_color field_enum;       /* TYPE_SCALAR (enum) */
    char field_char;            /* TYPE_SCALAR */
    unsigned int field_uint;    /* TYPE_SCALAR */
};

/* TYPE_ARRAY: Fixed-size array within structure */
struct GTY(()) gty_array_struct {
    int fixed_array[10];        /* TYPE_ARRAY */
    struct gty_basic_struct* GTY((length("len"))) var_array; /* TYPE_ARRAY with length */
    unsigned int len;
};

/* TYPE_POINTER: Pointer types */
typedef struct gty_basic_struct* GTY(()) gty_basic_ptr;
typedef struct gty_array_struct* GTY(()) gty_array_ptr;

/* Chain of structures for recursive traversal */
struct GTY(()) gty_linked_node {
    int value;
    struct gty_linked_node* GTY((skip)) next_skip;  /* Skip this pointer */
    struct gty_linked_node* GTY((null)) next_null;  /* Nullable pointer */
};

/* Global variables to ensure processing */
extern struct gty_basic_struct GTY(()) global_basic_struct;
extern struct gty_array_struct GTY(()) global_array_struct;

#endif /* GTY_TEST_BASIC_H */
