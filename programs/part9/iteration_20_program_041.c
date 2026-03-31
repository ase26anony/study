/* Base type definitions covering most switch cases */
%{
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%}

/* TYPE_SCALAR */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());

/* TYPE_STRING */
typedef const char * my_string_t GTY(());

/* TYPE_POINTER */
typedef my_scalar_t * my_scalar_ptr_t GTY(());

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;
typedef struct undefined_struct * undefined_ptr_t GTY(());

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_ulong_t field2;
  my_string_t field3 GTY((skip));  /* skip from GC */
  my_scalar_ptr_t field4;
  undefined_ptr_t field5;  /* TYPE_UNDEFINED pointer */
};

/* TYPE_UNION */
union my_union GTY((desc("tag")))
{
  int tag;
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_struct *as_struct GTY((tag("1")));
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  size_t length;
  struct my_struct *elements GTY((length("%h.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  my_scalar_t values[10];
};

/* Linked list using chain_next */
struct linked_node GTY(())
{
  my_scalar_t data;
  struct linked_node *next GTY((chain_next));
};

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(struct my_struct *data) GTY((callback));

/* Structure containing callback */
struct callback_container GTY(())
{
  my_callback_t callback;
  struct my_struct *data;
};
