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
typedef const char *my_string_t GTY(());

/* TYPE_POINTER */
typedef struct my_base_struct *my_pointer_t GTY(());

/* Forward declaration for TYPE_UNDEFINED case */
struct forward_declared GTY(());

/* TYPE_STRUCT */
struct my_base_struct GTY(()) {
  my_scalar_t id;
  my_string_t name;
  struct forward_declared *next;  /* Will be TYPE_UNDEFINED initially */
};

/* TYPE_ARRAY - variable length */
struct varray_struct GTY(()) {
  int length;
  my_scalar_t elements[1] GTY((length("%0.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array_struct GTY(()) {
  my_scalar_t fixed_elements[10];
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag"))) {
  int type_tag;
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_base_struct *as_struct GTY((tag("1")));
};

/* Now define the forward declared type */
struct forward_declared GTY(()) {
  int value;
  struct my_base_struct *link;
};
