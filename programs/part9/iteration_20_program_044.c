/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(void *data, int value);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_STRUCT with various fields */
struct GTY(()) my_base_struct {
  int id;
  const char *GTY((skip)) name;  /* skip from GC */
  struct my_base_struct *GTY((chain_next("%h.next"))) next;
  struct my_base_struct *GTY((chain_prev("%h.prev"))) prev;
};

/* TYPE_USER_STRUCT with custom marking */
struct GTY((user)) my_user_struct {
  void *custom_data;
  int ref_count;
};

/* TYPE_UNION with discriminator */
union GTY((desc("tag"))) my_union {
  int GTY((tag("0"))) as_int;
  double GTY((tag("1"))) as_double;
  struct my_base_struct *GTY((tag("2"))) as_struct;
  int tag;
};

/* TYPE_ARRAY examples */
struct GTY(()) array_container {
  int length;
  struct my_base_struct *GTY((length("%h.length"))) items[];
};

struct GTY(()) fixed_array_container {
  struct my_base_struct *GTY((length("10"))) fixed_items[10];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;
typedef my_callback_t *callback_ptr_t;

/* Complex nested structure */
struct GTY(()) complex_struct {
  struct my_base_struct *GTY((maybe_undef)) maybe_null;
  union my_union data;
  struct array_container *dynamic_array;
  my_callback_t callback;
  compare_func_t compare;
};

/* Linked list using chain_next */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((chain_next("%h.next"))) next;
};

/* Discriminated union structure */
struct GTY(()) tagged_union_container {
  int discriminator;
  union GTY((desc("%0.discriminator"))) {
    int GTY((tag("0"))) case_int;
    double GTY((tag("1"))) case_double;
    struct my_base_struct *GTY((tag("2"))) case_ptr;
  } data;
};

/* For TYPE_LANG_STRUCT - mimic tree node */
#ifdef TEST_LANG_STRUCT
struct GTY(()) c_tree_node {
  enum tree_code code;
  union tree_node *GTY((maybe_undef)) maybe_child;
  struct c_tree_node *GTY((chain_next)) siblings;
};
#endif

/* Template-like structure with param_is */
struct GTY((param_is(T))) template_container {
  void *GTY((skip)) data;
  int count;
};

/* Another forward declaration */
struct GTY(()) incomplete_struct;

/* Complete the forward declaration */
struct GTY(()) incomplete_struct {
  int completed;
  struct forward_declared_struct *forward_ref;
};

/* Complete the forward_declared_struct */
struct GTY(()) forward_declared_struct {
  int id;
  struct incomplete_struct *back_ref;
};

#endif /* TEST_GTYPE_H */
