/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#ifdef GENERATOR_FILE
#include "ansidecl.h"
#include "system.h"
#endif

/* Forward declarations to test TYPE_UNDEFINED */
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
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *GTY((skip)) next_skip;  /* Skip from GC */
  struct my_base_struct *GTY((chain_next("%h.next"))) next;
};

/* TYPE_USER_STRUCT with custom marking */
struct my_user_struct GTY((user))
{
  void *custom_data;
  int flags;
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag")))
{
  int type_tag;
  struct my_base_struct *GTY((tag("1"))) as_struct;
  my_scalar_t GTY((tag("2"))) as_scalar;
  my_string_t GTY((tag("3"))) as_string;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int count;
  struct my_base_struct * GTY((length("%h.count"))) items[1];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_base_struct *fixed_items[10];
  my_scalar_t numbers[5];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;
typedef my_callback_t *callback_ptr_t;

/* Linked list with chain_next/chain_prev */
struct linked_list GTY(())
{
  int value;
  struct linked_list *GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct linked_list *prev;
};

/* Nested structure for complex relationships */
struct nested_container GTY(())
{
  struct my_base_struct base;
  union my_union variant;
  struct array_container *array_ptr;
  my_callback_t callback;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void * GTY((skip)) data;
  int size;
};

/* Discriminated union with maybe_undef */
struct discriminated GTY(())
{
  int tag;
  union {
    struct my_base_struct *GTY((tag("0"))) base;
    struct forward_declared_struct *GTY((tag("1"), maybe_undef)) forward;
  } u;
};

#endif /* TEST_GTYPE_H */
