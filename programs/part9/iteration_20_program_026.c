/* Base type definitions covering most switch cases */
%{ #include "config.h" %}
%{ #include "system.h" %}
%{ #include "ansidecl.h" %}

/* TYPE_SCALAR */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());

/* TYPE_STRING */
typedef const char * my_string_t GTY(());

/* TYPE_POINTER */
typedef struct my_base_struct * my_ptr_t GTY(());

/* TYPE_ARRAY - variable length */
struct var_array_struct GTY(())
{
  int length;
  my_scalar_t *elements GTY((length("%0.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array_struct GTY(())
{
  my_scalar_t elements[10];
};

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *next GTY((skip));  /* skip from GC */
  struct my_base_struct *prev GTY((chain_prev("%0.next")));
  struct var_array_struct array_field;
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag")))
{
  int type_tag;
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_base_struct *as_struct GTY((tag("1")));
};

/* Forward declaration for TYPE_UNDEFINED test */
struct undefined_struct GTY(());

/* Struct that references undefined type */
struct uses_undefined GTY(())
{
  struct undefined_struct *undef_ptr GTY((maybe_undef));
  int valid;
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
  int value;
  struct uses_undefined *back_ref;
};
