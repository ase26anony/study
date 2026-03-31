/* test-gtypes.h - Comprehensive test for gengtype type coverage */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool boolean_type;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_type)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
  int id;
  const char *GTY((skip)) name;
  struct test_struct *next;
  callback_type callback;
};

/* TYPE_USER_STRUCT - user-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int int_val;
  double double_val;
  const char *string_val;
  struct test_struct *struct_ptr;
};

/* TYPE_ARRAY - within a struct */
struct GTY(()) array_container {
  int fixed_array[10];
  int GTY((length ("count"))) *variable_array;
  size_t count;
};

/* TYPE_POINTER - various pointer types */
struct GTY(()) pointer_examples {
  struct test_struct *direct_ptr;
  struct opaque_struct *opaque_ptr;
  void *generic_ptr;
  int *int_ptr;
  const char * const *string_ptr_ptr;
};

/* Linked list example for recursive structures */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((skip("next"))) next;
  struct linked_list *prev;
};

#endif /* TEST_GTYPES_H */
