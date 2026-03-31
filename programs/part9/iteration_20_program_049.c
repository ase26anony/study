/* Base type definitions covering most switch cases */
%{ /* Start of special directives */
#include "config.h"
#include "system.h"
#include "ansidecl.h"
%} /* End of special directives */

/* TYPE_SCALAR - Fundamental scalar types */
typedef int my_scalar_t GTY(());
typedef unsigned long my_unsigned_t GTY(());
typedef double my_float_t GTY(());

/* TYPE_STRING - String type */
typedef const char * my_string_t GTY(());

/* TYPE_UNDEFINED - Forward declaration */
struct undefined_struct GTY((maybe_undef));

/* TYPE_STRUCT - Basic structure */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_string_t field2 GTY((skip));  /* Skip from GC */
  struct undefined_struct *field3; /* Potentially undefined */
};

/* TYPE_UNION - Basic union */
union my_union GTY(())
{
  my_scalar_t int_val;
  my_float_t float_val;
  struct my_struct *struct_ptr GTY((tag("0")));
};

/* TYPE_POINTER - Pointer types */
typedef struct my_struct * my_struct_ptr GTY(());
typedef union my_union * my_union_ptr GTY(());

/* TYPE_ARRAY - Array types */
struct array_container GTY(())
{
  int length;
  struct my_struct *elements GTY((length("%h.length"))); /* Variable length */
  int fixed_array[10] GTY(()); /* Fixed length */
};

/* TYPE_USER_STRUCT - User-defined marking */
struct user_struct GTY((user))
{
  void *data;
  size_t size;
};

/* Linked list example with chain_next */
struct linked_node GTY(())
{
  int value;
  struct linked_node *next GTY((chain_next));
  struct linked_node *prev GTY((chain_prev));
};

/* Discriminated union */
struct tagged_union GTY(())
{
  enum { TAG_INT, TAG_FLOAT, TAG_STRUCT } tag;
  union {
    int int_val GTY((tag("TAG_INT")));
    double float_val GTY((tag("TAG_FLOAT")));
    struct my_struct *struct_ptr GTY((tag("TAG_STRUCT")));
  } u GTY((desc("%0.tag")));
};
