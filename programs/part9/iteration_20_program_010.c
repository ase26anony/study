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

/* Forward declarations for TYPE_UNDEFINED testing */
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *GTY((skip)) next_skip;  /* Skip from GC */
  struct my_base_struct *GTY((chain_next("%s.next"))) next_chain;
};

/* TYPE_UNION with discriminator */
struct my_discriminated_union GTY(())
{
  int tag;
  union {
    my_scalar_t as_scalar GTY((tag("0")));
    my_string_t as_string GTY((tag("1")));
    struct my_base_struct *GTY((tag("2"))) as_struct;
  } GTY((desc("%s.tag"))) u;
};

/* TYPE_ARRAY examples */
struct my_array_container GTY(())
{
  int length;
  struct my_base_struct *GTY((length("%s.length"))) variable_array[1];
  struct my_base_struct *fixed_array[10];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_struct_ptr;
typedef my_string_t *string_ptr_ptr;

/* Linked list structure for chain_next/prev testing */
struct my_linked_list GTY(())
{
  int value;
  struct my_linked_list *GTY((chain_next("%s.next"), chain_prev("%s.prev"))) next;
  struct my_linked_list *prev;
};

#endif /* TEST_GTYPE_BASE_H */
