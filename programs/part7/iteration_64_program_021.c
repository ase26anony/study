/* gty-test-basic.h - Basic GTY types for coverage testing */

#ifndef GTY_TEST_BASIC_H
#define GTY_TEST_BASIC_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef long GTY(()) scalar_long_t;
typedef char GTY(()) scalar_char_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum_t;

/* TYPE_STRUCT: Standard C structure */
struct GTY(()) basic_struct {
  scalar_int_t id;
  scalar_char_t initial;
  scalar_float_t value;
  color_enum_t color;
};

/* TYPE_ARRAY: Arrays of various types */
struct GTY(()) array_container {
  int GTY((length("count"))) *dynamic_array;
  int count;
  scalar_int_t GTY((skip)) fixed_array[10];
  struct basic_struct GTY(()) struct_array[5];
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct* GTY(()) struct_ptr_t;
typedef int* GTY((skip)) skip_ptr_t;  /* TYPE_POINTER with skip */
typedef void (*GTY(()) void_func_ptr)(void);

/* TYPE_STRING: String types */
struct GTY(()) string_container {
  const char* GTY((tag("0"))) constant_string;
  char* GTY((length("strlen"))) dynamic_string;
  int strlen;
};

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;  /* TYPE_UNDEFINED - never defined */

struct GTY(()) uses_opaque {
  struct opaque_struct* GTY(()) opaque_ptr;  /* Pointer to undefined type */
  void* GTY(()) void_ptr;  /* void pointer */
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_basic;
extern struct array_container GTY(()) global_array;
extern struct string_container GTY(()) global_strings;

#endif /* GTY_TEST_BASIC_H */
