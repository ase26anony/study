#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef double GTY(()) scalar_double_t;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum_t;

/* TYPE_STRUCT: Standard C structure */
struct GTY(()) basic_struct {
  scalar_int_t id;
  scalar_double_t value;
  color_enum_t color;
};

/* TYPE_ARRAY: Arrays within structures */
struct GTY(()) array_container {
  int GTY((length("count"))) *dynamic_array;
  int count;
  char GTY(()) fixed_array[100];
  struct basic_struct GTY(()) struct_array[10];
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_struct {
  struct basic_struct *GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED reference */
  struct basic_struct *GTY(()) gty_ptr;
  void *GTY(()) void_ptr;
  char *GTY(()) string_ptr;  /* TYPE_STRING */
  int (*GTY(()) func_ptr)(int, int);  /* TYPE_CALLBACK */
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_basic;
extern struct array_container GTY(()) global_array;
extern struct pointer_struct GTY(()) global_pointers;

#endif /* TEST_BASIC_STRUCTS_H */
