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
  char GTY((skip)) *name;         /* TYPE_POINTER with skip */
  double values[10];              /* TYPE_ARRAY */
  struct basic_struct *next;      /* TYPE_POINTER to same type */
};

/* TYPE_ARRAY: Array of structures */
typedef struct basic_struct GTY(()) basic_struct_array_t[5];

/* Global variable with GTY markup */
extern struct basic_struct GTY(()) *global_struct_list;

/* TYPE_STRING: String types */
typedef const char * GTY(()) const_string_t;
typedef char * GTY(()) mutable_string_t;

/* Structure with string fields */
struct GTY(()) string_container {
  const_string_t constant_string;  /* TYPE_STRING */
  mutable_string_t dynamic_string; /* TYPE_STRING */
  char fixed_string[50];           /* TYPE_ARRAY of char */
};

#endif /* TEST_BASIC_STRUCTS_H */
