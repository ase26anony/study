/* test-types.h - GCC GTY type definitions for coverage testing */
#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "ansidecl.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* Forward declaration for TYPE_UNDEFINED test */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *GTY((skip)) next_skip;  /* skip annotation */
  struct my_base_struct *GTY((chain_next("%h.next"))) next;
};

/* TYPE_USER_STRUCT with custom marking */
struct my_user_struct GTY((user))
{
  void *custom_data;
  int data_size;
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag")))
{
  int type_tag;
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_base_struct *GTY((tag("1"))) as_struct;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int count;
  struct my_base_struct *GTY((length("%h.count"))) items[1];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_base_struct *fixed_items[10];
  my_scalar_t scalar_array[5];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;

/* Nested structure for complex relationships */
struct complex_struct GTY(())
{
  struct array_container *container;
  union my_union current_union;
  base_ptr_t *pointer_array;  /* Pointer to pointer */
  struct my_user_struct *user_structs;
};

/* Linked list with chain_next/chain_prev */
struct linked_list GTY(())
{
  int value;
  struct linked_list *GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct linked_list *prev;
};

/* TYPE_CALLBACK */
typedef void (*my_callback_t) GTY((callback)) (struct my_base_struct *data, int status);

struct callback_container GTY(())
{
  my_callback_t callback;
  void *user_data;
};

/* Structure containing callback */
struct has_callback GTY(())
{
  my_callback_t notify;
  struct my_base_struct *data;
};

/* For param_is annotation */
struct param_struct GTY((param_is(T)))
{
  void *data;
  int size;
};

/* Maybe undefined type */
struct maybe_undefined GTY((maybe_undef))
{
  struct undefined_struct *ptr;
  int defined;
};

#endif /* TEST_TYPES_H */
