/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* Enum type for scalar coverage */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;           /* TYPE_SCALAR */
    char char_field;         /* TYPE_SCALAR */
    long long_field;         /* TYPE_SCALAR */
    float float_field;       /* TYPE_SCALAR */
    double double_field;     /* TYPE_SCALAR */
    color enum_field;        /* TYPE_SCALAR (enum) */
    bool bool_field;         /* TYPE_SCALAR (C++ bool) */
};

/* Structure with array fields - TYPE_ARRAY */
struct GTY(()) array_struct {
    int fixed_array[10];                 /* TYPE_ARRAY - fixed size */
    struct basic_struct *GTY((length("len"))) var_array; /* TYPE_ARRAY - variable length */
    int len;                             /* Length field for var_array */
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_struct {
    struct basic_struct *next;           /* TYPE_POINTER to struct */
    struct opaque_type *opaque_ptr;      /* TYPE_POINTER to undefined type */
    void *generic_ptr;                   /* TYPE_POINTER with void* */
    int (*func_ptr)(int, int);           /* TYPE_POINTER to function */
};

/* Global variables to ensure processing */
extern GTY(()) struct basic_struct global_basic;
extern GTY(()) struct array_struct global_array;
extern GTY(()) struct pointer_struct global_pointer;

/* String type - TYPE_STRING */
extern GTY(()) const char *global_string;
extern GTY(()) char *mutable_string;

#endif /* TEST_BASIC_STRUCTS_H */
