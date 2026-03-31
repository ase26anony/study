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

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *next GTY((skip));  /* skip from GC */
  struct my_base_struct *prev GTY((skip));
};

/* TYPE_UNION */
union my_discriminated_union GTY((desc("tag")))
{
  int tag;
  struct my_base_struct *as_struct GTY((tag("0")));
  my_string_t as_string GTY((tag("1")));
  my_scalar_t as_scalar GTY((tag("2")));
};

/* TYPE_ARRAY - variable length */
struct my_array_container GTY(())
{
  int length;
  struct my_base_struct *elements GTY((length("%h.length")));
};

/* TYPE_ARRAY - fixed length */
struct my_fixed_array GTY(())
{
  struct my_base_struct *fixed_elements[10];
};

/* TYPE_POINTER */
typedef struct my_base_struct *my_struct_ptr GTY(());

/* Forward declaration for TYPE_UNDEFINED test */
struct forward_declared GTY(());

/* Structure using forward declared type */
struct uses_forward_decl GTY(())
{
  struct forward_declared *fd_ptr;  /* Will be TYPE_UNDEFINED initially */
  int valid;
};

/* Now define the forward declared type */
struct forward_declared GTY(())
{
  int data;
  struct uses_forward_decl *back_ref;
};
