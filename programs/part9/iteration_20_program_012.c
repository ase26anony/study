/* Base type definitions covering most switch cases */
%{ /* Start of special directives */
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%}

/* TYPE_SCALAR definitions */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());
typedef double my_double_t GTY(());

/* TYPE_STRING definition */
typedef const char * my_string_t GTY(());

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct GTY((maybe_undef));

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  my_scalar_t count;
  my_string_t name;
  struct undefined_struct *undef_ptr GTY((skip)); /* TYPE_UNDEFINED pointer */
  struct my_struct *next GTY((chain_next("%h.next")));
};

/* TYPE_UNION with discriminator */
union my_union GTY((desc("tag")))
{
  int tag;
  struct my_struct *s_ptr GTY((tag("1")));
  my_scalar_t scalar_val GTY((tag("2")));
  my_string_t str_val GTY((tag("3")));
};

/* TYPE_ARRAY definitions */
struct array_container GTY(())
{
  int length;
  struct my_struct *variable_array GTY((length("%h.length")));
  my_scalar_t fixed_array[10] GTY(());
  struct my_struct *ptr_array[5] GTY(());
};

/* TYPE_POINTER in various forms */
typedef struct my_struct *my_struct_ptr GTY(());
typedef union my_union *my_union_ptr GTY(());
typedef my_scalar_t *scalar_ptr GTY(());

/* Linked list structure for chain_next/chain_prev */
struct linked_list GTY(())
{
  int data;
  struct linked_list *next GTY((chain_next("%h.next")));
  struct linked_list *prev GTY((chain_prev("%h.prev")));
};

/* Nested structure for complex traversal */
struct nested_container GTY(())
{
  struct my_struct inner_struct;
  union my_union inner_union;
  struct array_container array_cont;
  struct linked_list *list_head;
};
