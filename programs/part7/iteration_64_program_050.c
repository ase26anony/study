/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef unsigned int GTY(()) scalar_uint_t;

/* TYPE_ENUM (treated as scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};
typedef enum color GTY(()) color_t;

/* TYPE_STRUCT: Basic structure with multiple fields */
struct GTY(()) basic_struct {
    scalar_int_t id;                /* TYPE_SCALAR */
    scalar_char_t name[32];         /* TYPE_ARRAY of TYPE_SCALAR */
    scalar_float_t *GTY((skip)) ptr; /* TYPE_POINTER with skip */
    struct basic_struct *GTY((tag("0"))) next; /* TYPE_POINTER to TYPE_STRUCT */
    color_t color;                  /* TYPE_SCALAR (enum) */
};

/* TYPE_ARRAY: Various array types */
typedef struct basic_struct GTY(()) struct_array_t[10];
typedef int GTY(()) int_array_t[20];
typedef char * GTY(()) string_array_t[5];

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_struct;
extern struct_array_t GTY(()) global_struct_array;
extern int_array_t GTY(()) global_int_array;

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct GTY(()) opaque_struct;
extern struct opaque_struct *GTY(()) opaque_ptr;

#endif /* TEST_BASIC_STRUCTS_H */
