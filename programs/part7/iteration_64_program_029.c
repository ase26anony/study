/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* Basic scalar types - TYPE_SCALAR */
typedef int my_int_type GTY(());
typedef enum { RED, GREEN, BLUE } color_type GTY(());
typedef double my_double_type GTY(());

/* Simple structure - TYPE_STRUCT */
struct simple_struct GTY(())
{
  int scalar_field;                     /* TYPE_SCALAR */
  char char_field;                      /* TYPE_SCALAR */
  double double_field;                  /* TYPE_SCALAR */
  color_type enum_field;                /* TYPE_SCALAR (enum) */
};

/* Structure with arrays - TYPE_ARRAY */
struct array_struct GTY(())
{
  int fixed_array[10];                  /* TYPE_ARRAY (fixed size) */
  struct simple_struct struct_array[5]; /* TYPE_ARRAY of structs */
  char* string_array[3];                /* TYPE_ARRAY of TYPE_STRING */
};

/* Structure with pointers - TYPE_POINTER */
struct pointer_struct GTY(())
{
  struct simple_struct* direct_ptr;     /* TYPE_POINTER to struct */
  struct opaque_type* opaque_ptr;       /* TYPE_POINTER to TYPE_UNDEFINED */
  void* void_ptr;                       /* TYPE_POINTER with void */
  int* int_ptr;                         /* TYPE_POINTER to scalar */
  struct pointer_struct* self_ptr;      /* TYPE_POINTER to self */
};

/* Linked list structure for recursive traversal */
struct linked_list GTY(())
{
  int data;                            /* TYPE_SCALAR */
  struct linked_list* GTY((skip)) next_skip;  /* GTY((skip)) pointer */
  struct linked_list* GTY((tag("0"))) next;   /* TYPE_POINTER with tag */
};

/* Global variables to ensure processing */
extern struct simple_struct global_simple GTY(());
extern struct array_struct global_array GTY(());
extern struct pointer_struct* global_pointer_chain GTY(());

#endif /* TEST_BASIC_STRUCTS_H */
