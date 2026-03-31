/* Basic scalar types */
typedef int my_scalar_t GTY((tag("SCALAR")));
typedef unsigned long my_ulong_t GTY((tag("SCALAR")));

/* String type */
typedef const char * my_string_t GTY((tag("STRING")));

/* Forward declaration for undefined type testing */
struct undefined_struct GTY((maybe_undef));

/* Basic structure */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_ulong_t field2;
  my_string_t name GTY((skip));  /* skip annotation */
  struct undefined_struct *forward_ptr GTY((maybe_undef));
};

/* Union type */
union my_union GTY(())
{
  my_scalar_t as_int;
  my_ulong_t as_ulong;
  struct my_struct *as_struct GTY((tag("1")));
};

/* Pointer types */
typedef struct my_struct *my_struct_ptr GTY((tag("POINTER")));
typedef union my_union *my_union_ptr GTY((tag("POINTER")));

/* Array types */
struct my_array_struct GTY(())
{
  int length;
  struct my_struct *elements GTY((length("%h.length")));
};

/* Fixed-length array */
struct fixed_array_struct GTY(())
{
  struct my_struct fixed[10];
};

/* Linked list structure */
struct linked_list GTY(())
{
  int value;
  struct linked_list *next GTY((chain_next));
  struct linked_list *prev GTY((chain_prev));
};
