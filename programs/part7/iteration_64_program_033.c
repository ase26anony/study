/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* TYPE_SCALAR: Basic scalar types */
typedef int my_int_type GTY(());
typedef enum { RED, GREEN, BLUE } color_type GTY(());
typedef unsigned long size_type GTY(());

/* TYPE_STRUCT: Basic structure with multiple field types */
struct basic_struct GTY(())
{
  /* TYPE_SCALAR fields */
  int scalar_int;
  char scalar_char;
  double scalar_double;
  color_type scalar_enum;
  
  /* TYPE_ARRAY fields */
  int fixed_array[10];
  char string_array[256];
  
  /* TYPE_POINTER fields */
  struct basic_struct *next;  /* Self-referential pointer */
  char *dynamic_string;       /* TYPE_STRING */
  
  /* Pointer to undefined type */
  struct opaque_type *opaque_ptr;  /* TYPE_UNDEFINED in pointer context */
};

/* Global variable with GTY markup */
extern struct basic_struct *global_list GTY((root));

/* TYPE_ARRAY: Array of structures */
struct array_container GTY(())
{
  struct basic_struct elements[5];
  struct basic_struct *ptr_array[8];
};

#endif /* TEST_BASIC_STRUCTS_H */
