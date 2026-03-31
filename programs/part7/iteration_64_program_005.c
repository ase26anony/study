/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_forward_decl;

/* Basic scalar types */
typedef int GTY(()) my_int_type;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRUCT with scalar and array fields */
struct GTY(()) basic_struct {
  int scalar_int;
  long scalar_long;
  double scalar_double;
  char scalar_char;
  color_enum enum_field;
  
  /* TYPE_ARRAY */
  int fixed_array[10];
  char string_array[256];
  
  /* TYPE_POINTER */
  struct basic_struct *next;
  struct opaque_forward_decl *opaque_ptr;  /* TYPE_UNDEFINED reference */
  
  /* TYPE_STRING */
  const char *string_field;
  char *mutable_string;
};

/* Chain of structures for recursive traversal */
struct GTY(()) linked_list {
  int data;
  struct linked_list *GTY((skip)) skip_ptr;  /* GTY((skip)) option */
  struct linked_list *next;
};

/* Global variables to ensure processing */
extern struct basic_struct GTY(()) global_struct;
extern struct linked_list *GTY(()) global_list;

#endif /* TEST_BASIC_STRUCTS_H */
