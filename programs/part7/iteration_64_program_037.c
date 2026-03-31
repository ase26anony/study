/* test-basic-structs.h - Cover TYPE_STRUCT, TYPE_SCALAR, TYPE_ARRAY, TYPE_POINTER */

#ifndef TEST_BASIC_STRUCTS_H
#define TEST_BASIC_STRUCTS_H

#include "config.h"
#include "system.h"

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_type;  /* TYPE_UNDEFINED - incomplete type */

/* Basic scalar types */
typedef int my_int GTY(());
typedef unsigned long my_ulong GTY(());
typedef enum { RED, GREEN, BLUE } color_enum GTY(());
typedef double my_double GTY(());

/* TYPE_STRUCT with scalar fields */
struct basic_struct GTY(())
{
  int field1;
  long field2;
  double field3;
  float field4;
  char field5;
  bool field6;  /* C++ bool, but gengtype handles it */
  color_enum field7;
};

/* TYPE_ARRAY - fixed size array */
struct array_container GTY(())
{
  int numbers[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  struct basic_struct structs[5];  /* TYPE_ARRAY of TYPE_STRUCT */
  char* strings[3];  /* TYPE_ARRAY of TYPE_STRING */
};

/* TYPE_POINTER fields */
struct pointer_struct GTY(())
{
  struct basic_struct* next GTY((skip));  /* TYPE_POINTER with skip */
  struct array_container* container GTY(());
  void* opaque_ptr;  /* TYPE_POINTER to TYPE_UNDEFINED (void) */
  int* int_ptr GTY(());
};

/* Chain of structures for recursive traversal */
struct linked_node GTY(())
{
  int data;
  struct linked_node* next GTY(());
  struct linked_node* prev GTY(());
};

/* Global variables to ensure processing */
extern struct basic_struct global_struct GTY(());
extern struct array_container global_array GTY(());
extern struct linked_node* global_list GTY(());

#endif /* TEST_BASIC_STRUCTS_H */
