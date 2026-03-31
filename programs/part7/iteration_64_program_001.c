/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t GTY((user));

/* Basic scalar types - TYPE_SCALAR */
typedef int my_int GTY(());
typedef unsigned long my_ulong GTY(());
typedef double my_double GTY(());
typedef enum { RED, GREEN, BLUE } color_t GTY(());

/* Simple structure - TYPE_STRUCT */
struct simple_struct GTY(())
{
  my_int field1;
  my_ulong field2;
  my_double field3;
  color_t field4;
};

/* Structure with arrays - TYPE_ARRAY */
struct array_struct GTY(())
{
  int fixed_array[10] GTY(());
  char *string_array[5] GTY(());
  struct simple_struct struct_array[3] GTY(());
};

/* Structure with pointers - TYPE_POINTER */
struct pointer_struct GTY(())
{
  struct simple_struct *next GTY(());
  struct array_struct *prev GTY(());
  void *generic_ptr GTY((skip));
  opaque_ptr_t opaque_ptr;
};

/* Chain of structures for recursive traversal */
struct linked_node GTY(())
{
  int data;
  struct linked_node *next GTY(());
  struct linked_node *prev GTY(());
};

/* Global variables to ensure processing */
extern struct simple_struct global_simple GTY(());
extern struct array_struct global_array GTY(());
extern struct pointer_struct global_pointer GTY(());
extern struct linked_node *global_list GTY(());

#endif /* TEST_BASIC_STRUCTS_H */
