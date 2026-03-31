/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  my_scalar_t scalar_field;
  my_ulong_t ulong_field;
  struct my_struct *GTY((skip)) next_skip;  /* skip annotation */
  struct my_struct *GTY((chain_next("%h.next"))) next_chain;
  my_string_t name;
  struct undefined_struct *GTY((maybe_undef)) undef_ptr;  /* may be undefined */
};

/* TYPE_USER_STRUCT with custom marking */
struct user_struct GTY((user))
{
  int id;
  void *custom_data;
};

/* TYPE_UNION */
union my_union GTY((desc("tag")))
{
  int tag;
  struct my_struct *GTY((tag("1"))) struct_ptr;
  my_scalar_t GTY((tag("2"))) scalar_val;
  my_string_t GTY((tag("3"))) string_val;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int length;
  struct my_struct *GTY((length("%h.length"))) variable_array[1];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_struct *GTY((skip)) fixed[10];
};

/* TYPE_POINTER to various types */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef struct user_struct *user_struct_ptr;

/* TYPE_CALLBACK */
typedef void (*my_callback_t) GTY((callback)) (struct my_struct *arg);

/* Callback structure */
struct callback_container GTY(())
{
  my_callback_t callback;
  struct my_struct *context;
};

/* Linked list structure for chain_next/prev */
struct linked_list GTY(())
{
  int value;
  struct linked_list *GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct linked_list *prev;
};

/* Nested structure */
struct outer_struct GTY(())
{
  struct my_struct inner;
  union my_union variant;
  struct array_container arrays;
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
  int defined_now;
  struct my_struct *ptr_to_struct;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void *GTY((skip)) data;
  int length;
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
struct c_tree_node GTY(())
{
  int code;
  struct c_tree_node *GTY((skip)) left;
  struct c_tree_node *right;
};
#endif

#endif /* TEST_GTYPE_H */
