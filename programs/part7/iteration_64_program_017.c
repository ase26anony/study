/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_forward_decl;

/* Basic scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef double GTY(()) scalar_double_t;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum_t;

/* Simple structure - TYPE_STRUCT */
struct GTY(()) simple_struct {
  scalar_int_t id;
  scalar_double_t value;
  color_enum_t color;
};

/* Structure with arrays - TYPE_ARRAY */
struct GTY(()) array_struct {
  int GTY((length("array_length"))) *dynamic_array;
  int array_length;
  
  /* Fixed-size array */
  char GTY(()) fixed_array[32];
  
  /* Array of pointers */
  struct simple_struct* GTY(()) ptr_array[10];
};

/* Structure with pointers - TYPE_POINTER */
struct GTY(()) pointer_struct {
  struct simple_struct* GTY(()) next;
  struct array_struct* GTY(()) data;
  
  /* Pointer to undefined type - TYPE_UNDEFINED in pointer context */
  struct opaque_forward_decl* GTY(()) opaque_ptr;
  
  /* Self-referential pointer */
  struct pointer_struct* GTY(()) self;
};

/* Global variables to ensure processing */
extern struct simple_struct GTY(()) global_simple;
extern struct array_struct GTY(()) global_array;
extern struct pointer_struct* GTY(()) global_pointer_chain;

#endif /* TEST_BASIC_STRUCTS_H */
