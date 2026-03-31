/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

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
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
  int id;
  const char *GTY((skip)) name;
  struct test_struct *next;
};

/* TYPE_USER_STRUCT - user-defined struct */
typedef struct GTY((user)) user_struct {
  long custom_data;
  void *user_context;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int as_int;
  float as_float;
  void *as_pointer;
  struct test_struct *as_struct;
};

/* TYPE_ARRAY types */
struct GTY(()) array_container {
  int fixed_array[10];
  int *GTY((length("len"))) variable_array;
  size_t len;
};

/* Linked list for recursive testing */
struct GTY(()) linked_node {
  int value;
  struct linked_node *GTY((skip("next"))) next;
  struct linked_node *prev;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  struct test_struct base;
  union test_union choice;
  struct array_container arrays;
  callback_type callback;
};

#endif /* TEST_GTYPES_H */
