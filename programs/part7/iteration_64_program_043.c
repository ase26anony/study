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

/* TYPE_STRUCT: Basic structure with multiple scalar fields */
struct GTY(()) basic_struct {
  scalar_int_t id;
  scalar_long_t count;
  scalar_double_t value;
  color_enum_t color;
};

/* TYPE_ARRAY: Arrays within structures */
struct GTY(()) array_container {
  int GTY((length("len"))) *dynamic_array;
  unsigned int len;
  
  /* Fixed-size array */
  scalar_int_t GTY(()) fixed_array[10];
  
  /* Array of pointers */
  struct basic_struct* GTY((skip)) *ptr_array[5];
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) basic_struct_ptr;
typedef struct array_container* GTY(()) array_container_ptr;

/* Chain of structures for recursive traversal */
struct GTY(()) linked_node {
  scalar_int_t data;
  struct linked_node* GTY((skip)) next;  /* Skip this pointer */
  struct linked_node* GTY(()) prev;      /* Tracked pointer */
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_struct;
extern struct array_container GTY(()) global_array_container;

#endif /* TEST_BASIC_STRUCTS_H */
