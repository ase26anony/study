/* test-gtypes.h - Comprehensive type definitions for gengtype coverage */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR - Basic scalar types */
typedef int scalar_int_t;
typedef enum { RED, GREEN, BLUE } color_enum_t;
typedef bool bool_t;

/* TYPE_STRING */
typedef const char *string_t;

/* TYPE_CALLBACK - Function pointer type */
typedef void (*callback_func_t)(void *data);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_STRUCT - Standard struct */
struct GTY(()) base_struct {
  int id;
  string_t name;
  color_enum_t color;
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
struct user_struct;
typedef struct user_struct *user_ptr_t GTY((user));

/* TYPE_UNION */
union GTY(()) data_union {
  int int_val;
  double double_val;
  string_t string_val;
  struct base_struct *struct_ptr;
};

/* TYPE_ARRAY - Fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];
  struct base_struct *GTY((length("len"))) variable_array;
  size_t len;
};

/* TYPE_POINTER - Linked list structure */
struct GTY(()) linked_node {
  int data;
  struct linked_node *GTY((skip)) next;
  struct linked_node *prev;
};

/* Recursive structure with multiple pointer types */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((skip)) left;
  struct tree_node *right;
  union data_union data;
};

/* Complex nested structure */
struct GTY(()) complex_struct {
  struct base_struct base;
  union data_union union_field;
  struct array_container array_field;
  callback_func_t callback;
  string_t description;
};

#endif /* TEST_GTYPES_H */
