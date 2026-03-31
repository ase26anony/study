/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* This will be TYPE_UNDEFINED */

/* Enum type for scalar coverage */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* Basic structure with scalar fields - TYPE_STRUCT */
struct GTY(()) basic_struct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    color_t enum_field;     /* TYPE_SCALAR (enum) */
    bool bool_field;        /* TYPE_SCALAR (C++ bool) */
};

/* Structure with array fields - TYPE_ARRAY */
struct GTY(()) array_struct {
    int fixed_array[10];           /* Fixed-size array */
    char* GTY((length("len"))) variable_array;  /* Variable-length array */
    size_t len;                    /* Length field for variable array */
    
    /* Nested array of pointers */
    struct basic_struct* GTY((skip)) ptr_array[5];
};

/* Structure with pointer fields - TYPE_POINTER */
struct GTY(()) pointer_struct {
    struct basic_struct* direct_ptr;      /* Pointer to struct */
    struct opaque_type* undefined_ptr;    /* Pointer to undefined type */
    void* void_ptr;                       /* void pointer */
    
    /* Chain of pointers for recursive traversal */
    struct pointer_struct* GTY((tag("0"))) next;
};

/* Global variables to ensure processing */
extern GTY(()) struct basic_struct global_basic;
extern GTY(()) struct array_struct global_array;
extern GTY(()) struct pointer_struct* global_chain;

#endif /* TEST_BASIC_STRUCTS_H */
