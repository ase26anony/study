/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_undefined;

/* TYPE_SCALAR - Basic scalar types */
typedef int scalar_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef bool scalar_bool;

/* TYPE_STRING */
typedef const char *string_type;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - Standard struct */
struct GTY(()) test_struct {
  int id;
  const char *name;
  struct test_struct *next;
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) test_union {
  int int_val;
  double double_val;
  const char *string_val;
  struct test_struct *struct_ptr;
};

/* TYPE_POINTER - Various pointer types */
typedef struct test_struct *struct_ptr;
typedef union test_union *union_ptr;
typedef callback_func *callback_ptr;

/* TYPE_ARRAY - Array types */
struct GTY(()) array_container {
  int fixed_array[10];
  struct test_struct *variable_array GTY((length("count")));
  int count;
};

/* Nested and recursive structures for deep processing */
struct GTY(()) complex_node {
  int value;
  struct complex_node *left;
  struct complex_node *right;
  union test_union data;
  callback_func callback;
};

/* Linked list structure */
struct GTY(()) linked_list {
  int data;
  struct linked_list *next;
  struct linked_list *prev;
};

#endif /* TEST_GTYPES_H */
