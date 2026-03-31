/* test-gtype-base.h - Base type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_BASE_H
#define TEST_GTYPE_BASE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR definitions */
typedef int my_scalar_t;
typedef unsigned long my_unsigned_scalar_t;

/* TYPE_STRING definition */
typedef const char *my_string_t;

/* TYPE_CALLBACK definition */
typedef void (*my_callback_t)(void *data, int value);
typedef int (*compare_callback_t)(const void *, const void *);

/* TYPE_USER_STRUCT with user-defined marking */
struct GTY((user)) user_defined_struct {
  void *data;
  size_t size;
};

/* TYPE_STRUCT with various field types */
struct GTY(()) my_base_struct {
  int GTY((skip)) private_field;  /* Skip from GC */
  my_scalar_t scalar_field;
  my_string_t string_field;
  struct my_base_struct *GTY((tag("0"))) next;  /* Pointer field */
  struct my_base_struct *GTY((chain_next("%h.next"))) chain_next_field;
  struct my_base_struct *GTY((chain_prev("%h.next"))) chain_prev_field;
};

/* TYPE_UNION definition */
union GTY((desc("%0.type_tag"))) my_discriminated_union {
  int type_tag;
  struct {
    int int_value;
    my_scalar_t scalar_value;
  } GTY((tag("1"))) int_case;
  struct {
    double double_value;
    my_string_t string_value;
  } GTY((tag("2"))) double_case;
  struct my_base_struct *GTY((tag("3"))) struct_ptr_case;
};

/* TYPE_ARRAY definitions */
struct GTY(()) array_container {
  int count;
  struct my_base_struct *GTY((length("%h.count"))) variable_array[1];
  int GTY(()) fixed_array[10];
  my_string_t GTY((length("5"))) string_array[5];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_struct_ptr;
typedef union my_discriminated_union *union_ptr;
typedef my_callback_t *callback_ptr;

/* Nested structure for complex relationships */
struct GTY(()) complex_container {
  struct my_base_struct *GTY(()) base_ptr;
  union my_discriminated_union GTY(()) union_field;
  struct array_container *GTY(()) array_ptr;
  my_callback_t GTY((callback)) callback_field;
  struct user_defined_struct *GTY(()) user_struct_ptr;
};

/* Linked list structure using chain_next */
struct GTY(()) linked_list_node {
  int data;
  struct linked_list_node *GTY((chain_next("%h.next"))) next;
  struct linked_list_node *GTY((chain_prev("%h.next"))) prev;
};

#endif /* TEST_GTYPE_BASE_H */
