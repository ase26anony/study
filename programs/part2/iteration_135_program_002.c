/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

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
typedef struct GTY((user)) user_struct {
  int user_id;
  void * GTY((skip)) user_data;  /* Skip for GC */
  callback_func_t callback;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) data_union {
  int int_val;
  double double_val;
  string_t string_val;
  struct base_struct * GTY((tag("0"))) struct_ptr;
};

/* TYPE_POINTER - Various pointer types */
typedef struct base_struct *base_ptr_t;
typedef user_struct_t *user_ptr_t;
typedef opaque_ptr_t opaque_pointer_t;

/* TYPE_ARRAY - Fixed-size array */
struct GTY(()) array_container {
  int fixed_array[10];
  struct base_struct * GTY((length("len"))) var_array;
  size_t len;
};

/* Recursive structure for deep processing */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  struct linked_node * GTY((skip)) prev;
  union data_union data;
};

/* Complex nested structure */
struct GTY(()) complex_type {
  struct base_struct base;
  user_struct_t user;
  union data_union union_field;
  struct array_container array_field;
  struct linked_node *node_list;
  callback_func_t handlers[5];
  string_t strings[3];
};

#endif /* TEST_GTYPES_H */
