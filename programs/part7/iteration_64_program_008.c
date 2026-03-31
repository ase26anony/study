#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef GTY(()) int my_int;
typedef GTY(()) long my_long;
typedef GTY(()) char my_char;
typedef GTY(()) float my_float;
typedef GTY(()) double my_double;

/* TYPE_ENUM (part of TYPE_SCALAR) */
typedef enum GTY(()) {
    STATE_A,
    STATE_B,
    STATE_C
} my_enum;

/* TYPE_STRUCT: Basic structure with scalar fields */
struct GTY(()) basic_struct {
    my_int field1;
    my_long field2;
    my_char field3;
    my_float field4;
    my_double field5;
    my_enum field6;
};

/* TYPE_ARRAY: Arrays within structures */
struct GTY(()) array_struct {
    int GTY((length("10"))) fixed_array[10];
    char* GTY((length("len"))) variable_array;
    int len;
};

/* TYPE_POINTER: Pointer fields */
struct GTY(()) pointer_struct {
    struct basic_struct* GTY((skip)) next;  /* Skip this pointer */
    struct array_struct* GTY(()) data;
    void* GTY((tag("0"))) generic_ptr;
};

/* Global variables to ensure processing */
extern GTY(()) struct basic_struct global_basic;
extern GTY(()) struct array_struct global_array;
extern GTY(()) struct pointer_struct global_pointer;

#endif /* TEST_BASIC_STRUCTS_H */
