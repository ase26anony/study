/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef double GTY(()) scalar_double_t;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum_t;

/* TYPE_STRUCT: Basic structure with multiple fields */
struct GTY(()) basic_struct {
  scalar_int_t id;                /* TYPE_SCALAR */
  char GTY((skip)) *name;         /* TYPE_STRING with skip */
  scalar_double_t value;          /* TYPE_SCALAR */
  color_enum_t color;             /* TYPE_SCALAR (enum) */
};

/* TYPE_ARRAY: Fixed-size array within structure */
struct GTY(()) array_container {
  int GTY((length("count"))) *dynamic_array;  /* TYPE_ARRAY with length */
  int count;
  struct basic_struct GTY((tag("0"))) fixed_array[10];  /* TYPE_ARRAY fixed */
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr_t;
typedef int* GTY(()) int_ptr_t;
typedef void (*GTY(()) func_ptr_t)(void);  /* TYPE_POINTER to function */

/* Chain of structures for recursive traversal */
struct GTY(()) linked_node {
  int data;
  struct linked_node* GTY((skip)) next_skip;  /* TYPE_POINTER with skip */
  struct linked_node* GTY(()) next;           /* TYPE_POINTER */
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_struct;
extern struct array_container GTY(()) global_array_container;

#endif /* TEST_BASIC_STRUCTS_H */
