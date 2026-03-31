/* Base type definitions covering most switch cases */

/* TYPE_SCALAR */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());

/* TYPE_STRING */
typedef const char *my_string_t GTY(());

/* TYPE_POINTER */
typedef my_scalar_t *my_scalar_ptr_t GTY(());

/* Forward declaration for TYPE_UNDEFINED case */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_string_t field2;
  my_scalar_ptr_t field3;
  struct forward_declared_struct *field4 GTY((skip)); /* Will be TYPE_UNDEFINED initially */
  struct my_struct *next GTY((chain_next));
};

/* TYPE_UNION */
union my_union GTY(())
{
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_struct *as_struct GTY(%);
  int tag_field;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int length;
  struct my_struct *elements GTY((length("%h.length")));
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_struct fixed_elements[10];
};

/* TYPE_UNDEFINED becomes defined */
struct forward_declared_struct GTY(())
{
  my_scalar_t value;
  struct my_struct *link GTY(%);
};
