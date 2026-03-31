/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - Basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_type)(int, void*);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT - Standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;
  struct base_struct *GTY((skip)) next;
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int int_val;
  double double_val;
  struct base_struct *GTY((skip)) struct_ptr;
  const char *GTY((skip)) string_val;
};

/* TYPE_ARRAY - Various array types */
struct GTY(()) array_container {
  int fixed_array[10];
  int GTY((length ("%h.count"))) *variable_array;
  unsigned int count;
  struct base_struct *GTY((length ("%h.count"))) *struct_array;
};

/* TYPE_POINTER - Various pointer types */
struct GTY(()) pointer_container {
  struct base_struct *GTY((skip)) direct_ptr;
  struct base_struct **GTY((skip)) ptr_to_ptr;
  void *GTY((skip)) generic_ptr;
  callback_type GTY((skip)) callback_ptr;
};

/* Linked list for recursive testing */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((skip)) next;
  struct linked_list *GTY((skip)) prev;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  union test_union data;
  struct array_container arrays;
  struct pointer_container pointers;
  callback_type GTY((skip)) handlers[5];
};

#endif /* TEST_GTYPES_H */
