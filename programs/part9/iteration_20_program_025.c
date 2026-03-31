/* test-base.gtype - Base type definitions for gengtype coverage testing */

/* TYPE_SCALAR - Fundamental scalar types */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());
typedef double my_double_t GTY(());

/* TYPE_STRING - String type */
typedef const char * my_string_t GTY(());

/* TYPE_UNDEFINED - Forward declaration to create undefined type */
struct undefined_struct;
typedef struct undefined_struct * undefined_ptr_t GTY(());

/* TYPE_STRUCT - Basic structure with various field types */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_ulong_t field2;
  my_string_t field3 GTY((skip));  /* Skip from GC */
  struct my_struct *next GTY((chain_next));  /* Linked list */
};

/* TYPE_POINTER - Pointer types */
typedef struct my_struct * my_struct_ptr_t GTY(());
typedef my_scalar_t * scalar_ptr_t GTY(());

/* TYPE_ARRAY - Array types */
struct array_container GTY(())
{
  int length;
  struct my_struct *elements GTY((length("%0.length")));  /* Variable-length array */
  my_scalar_t fixed_array[10] GTY(());  /* Fixed-length array */
};

/* TYPE_UNION - Union type */
union my_union GTY((desc("tag")))
{
  int tag;
  struct my_struct *as_struct GTY((tag("1")));
  my_scalar_t *as_scalar GTY((tag("2")));
  my_string_t as_string GTY((tag("3")));
};

/* TYPE_USER_STRUCT - Structure with user-defined marking */
struct user_defined GTY((user))
{
  void *custom_data;
  int custom_size;
};

/* TYPE_CALLBACK - Callback function pointer */
typedef void (*my_callback_t)(struct my_struct *data, int value) GTY((callback));

/* Complex nested structure to test traversal */
struct complex_nested GTY(())
{
  struct my_struct base;
  union my_union variant;
  struct array_container arrays;
  my_callback_t callback;
  struct complex_nested *children GTY((length("child_count")));
  int child_count;
  struct user_defined *user_data GTY(());
};

/* Linked list structure for chain_next/chain_prev */
struct linked_list GTY(())
{
  int value;
  struct linked_list *next GTY((chain_next));
  struct linked_list *prev GTY((chain_prev));
};
