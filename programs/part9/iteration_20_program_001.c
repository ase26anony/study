/* base-types.gtype - Basic type definitions for gengtype testing */

/* TYPE_SCALAR - Fundamental scalar types */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());
typedef double my_double_t GTY(());

/* TYPE_STRING - String type */
typedef const char * my_string_t GTY(());

/* TYPE_UNDEFINED - Forward declaration */
struct undefined_struct;

/* TYPE_STRUCT - Basic structure */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_ulong_t field2;
  my_string_t field3 GTY((skip));  /* Skip from GC */
  struct undefined_struct *field4 GTY((maybe_undef)); /* Potentially undefined */
};

/* TYPE_POINTER - Pointer types */
typedef struct my_struct * my_struct_ptr GTY(());
typedef my_scalar_t * scalar_ptr GTY(());

/* TYPE_ARRAY - Array types */
struct array_container GTY(())
{
  int length;
  struct my_struct *elements GTY((length("%0.length"))); /* Variable length array */
  my_scalar_t fixed_array[10] GTY(()); /* Fixed length array */
};

/* TYPE_UNION - Union type */
union my_union GTY((desc("tag")))
{
  int tag;
  struct my_struct *as_struct GTY((tag("1")));
  my_scalar_t *as_scalar GTY((tag("2")));
  my_string_t as_string GTY((tag("3")));
};

/* TYPE_USER_STRUCT - User-defined marking */
struct user_struct GTY((user))
{
  void *custom_data;
  int custom_flag;
};

/* Linked list example with chain_next */
struct linked_node GTY(())
{
  int value;
  struct linked_node *next GTY((chain_next));
  struct linked_node *prev GTY((chain_prev));
};

/* Nested structure for complex relationships */
struct container GTY(())
{
  struct my_struct inner;
  union my_union variant;
  struct user_struct *user_data;
  struct linked_node *list_head;
  struct array_container arrays;
};
