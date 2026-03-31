/* test-gtypes.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR - basic scalar types */
typedef int my_scalar_type;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING */
typedef const char *filename_type;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_STRUCT - standard struct */
struct GTY(()) base_struct {
  int id;
  const char *GTY((skip)) name;  /* TYPE_STRING */
  struct base_struct *next;      /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct GTY((user)) user_struct {
  int user_data;
  void *GTY((skip)) user_ptr;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) data_union {
  int int_val;
  double double_val;
  struct base_struct *GTY((tag("0"))) struct_ptr;
};

/* TYPE_ARRAY - various array types */
struct GTY(()) array_container {
  int fixed_array[10];                     /* fixed-size array */
  struct base_struct *GTY((length("len"))) var_array; /* variable-length array */
  size_t len;
  color_enum color_array[5];               /* array of scalar enum */
};

/* TYPE_POINTER - pointer types */
typedef struct base_struct *base_ptr;
typedef union data_union *union_ptr;
typedef struct array_container **double_ptr;

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((skip("left"))) left;
  struct tree_node *GTY((skip("right"))) right;
  union data_union data;
};

/* Complex nested structure */
struct GTY(()) complex_type {
  struct base_struct base;
  union data_union union_field;
  struct array_container array_field;
  callback_func callback;  /* TYPE_CALLBACK */
  filename_type filename;  /* TYPE_STRING */
  color_enum color;        /* TYPE_SCALAR */
};

#endif /* TEST_GTYPES_H */
