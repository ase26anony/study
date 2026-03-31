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

/* Basic structure with multiple scalar types - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;           /* TYPE_SCALAR */
    char char_field;         /* TYPE_SCALAR */
    long long_field;         /* TYPE_SCALAR */
    float float_field;       /* TYPE_SCALAR */
    double double_field;     /* TYPE_SCALAR */
    color_t enum_field;      /* TYPE_SCALAR (enum) */
    unsigned char byte_field; /* TYPE_SCALAR */
};

/* Structure with arrays - TYPE_ARRAY */
struct GTY(()) array_struct {
    int fixed_array[10];                 /* Fixed-size array */
    char* GTY((length("strlen(%h.str_field) + 1"))) str_field; /* Variable-length string */
    struct basic_struct* GTY((skip)) ptr_array[5]; /* Array of pointers */
};

/* Structure with pointers - TYPE_POINTER */
struct GTY(()) pointer_struct {
    struct basic_struct* GTY((tag("0"))) direct_ptr;     /* Direct pointer */
    struct opaque_forward_decl* GTY((skip)) opaque_ptr; /* Pointer to undefined type */
    void* GTY((skip)) void_ptr;                         /* void pointer */
    int (*func_ptr)(int, char*);                       /* Function pointer */
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_basic;
extern struct array_struct GTY(()) global_array;
extern struct pointer_struct GTY(()) global_pointer;

#endif /* TEST_BASIC_STRUCTS_H */
