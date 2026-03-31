/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_undefined;

/* TYPE_SCALAR - basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } scalar_enum;
typedef bool scalar_bool;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_type)(int, void*);

/* TYPE_STRUCT - standard struct */
struct GTY(()) test_struct {
  int id;
  const char * GTY((skip)) name;
  struct test_struct *next;
};

/* TYPE_USER_STRUCT - user-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int custom_field;
  void * GTY((skip)) user_data;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int as_int;
  double as_double;
  void *as_pointer;
};

/* TYPE_ARRAY types */
struct GTY(()) array_container {
  int fixed_array[10];
  int * GTY((length("len"))) variable_array;
  size_t len;
};

/* TYPE_POINTER - standalone pointer type */
typedef struct test_struct *struct_pointer;

/* Linked list for recursive testing */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip("next"))) next;
  struct linked_node *prev;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  union test_union data;
  struct array_container arrays;
  callback_type callback;
  string_type description;
};

#endif /* TEST_GTYPES_H */
