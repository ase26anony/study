/* Base type definitions covering most switch cases */
%{ /* Start of gengtype directives */
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%}

/* TYPE_SCALAR definitions */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING definition */
typedef const char *my_string_t;

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  int scalar_field;
  unsigned long ulong_field;
  const char * GTY((skip)) skipped_string;
  struct undefined_struct *forward_ptr;
};

/* TYPE_UNION with discriminator */
union my_discriminated_union GTY((desc("tag")))
{
  int tag;
  struct my_base_struct * GTY((tag("1"))) as_struct;
  int GTY((tag("2"))) as_int;
  const char * GTY((tag("3"))) as_string;
};

/* TYPE_ARRAY definitions */
struct my_array_container GTY(())
{
  int length;
  struct my_base_struct * GTY((length("%h.length"))) variable_array[1];
  int fixed_array[10];
  union my_discriminated_union union_array[5];
};

/* TYPE_POINTER with chain links */
struct my_linked_list GTY(())
{
  int data;
  struct my_linked_list * GTY((chain_next("%h.next"))) next;
  struct my_linked_list * GTY((chain_prev("%h.prev"))) prev;
};

/* Nested structures for complex relationships */
struct complex_container GTY(())
{
  struct my_base_struct base;
  union my_discriminated_union discriminator;
  struct my_linked_list *list_head;
  struct my_array_container *array_ptr;
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
  int defined_now;
  struct complex_container *container_ptr;
};
