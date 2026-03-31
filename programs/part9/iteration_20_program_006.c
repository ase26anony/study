/* test-gtype-base.h - Base type definitions for gengtype coverage testing */

#ifndef TEST_GTYPE_BASE_H
#define TEST_GTYPE_BASE_H

#include "ansidecl.h"
#include "system.h"

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* Forward declaration for TYPE_UNDEFINED testing */
struct forward_declared_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *GTY((skip)) next_skip;  /* Skip from GC */
  struct my_base_struct *GTY((chain_next("%h.next"))) next;
};

/* TYPE_UNION */
union my_base_union GTY((desc("type_tag")))
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
  struct my_base_struct *GTY((length("%h.count"))) items[];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_base_struct *GTY((length("10"))) fixed_items[10];
};

/* TYPE_POINTER with chain operations */
struct linked_list GTY(())
{
  int value;
  struct linked_list *GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
  struct linked_list *prev;
};

#endif /* TEST_GTYPE_BASE_H */
