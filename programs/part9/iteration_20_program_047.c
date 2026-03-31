/* base-types.gtype - Basic type definitions for gengtype testing */

/* TYPE_SCALAR - Fundamental scalar types */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());
typedef double my_double_t GTY(());

/* TYPE_STRING - String type */
typedef const char * my_string_t GTY(());

/* TYPE_UNDEFINED - Forward declaration to create undefined type */
struct undefined_struct;
typedef struct undefined_struct * undefined_ptr_t GTY(());

/* TYPE_STRUCT - Basic structure */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_string_t field2 GTY((skip));  /* Skip from GC */
  struct my_struct *next GTY((chain_next));  /* Linked list */
};

/* TYPE_UNION - Basic union */
union my_union GTY((desc("tag")))
{
  my_scalar_t as_scalar;
  my_string_t as_string;
  struct my_struct *as_struct;
  int tag;
};

/* TYPE_POINTER - Various pointer types */
typedef struct my_struct * struct_ptr_t GTY(());
typedef union my_union * union_ptr_t GTY(());
typedef my_scalar_t * scalar_ptr_t GTY(());

/* TYPE_ARRAY - Array types */
struct array_container GTY(())
{
  struct my_struct *variable_array GTY((length("var_len")));
  my_scalar_t fixed_array[10];
  int var_len;
};

/* TYPE_USER_STRUCT - User-defined marking */
struct user_struct GTY((user))
{
  void *custom_data;
  int custom_flag;
};

/* TYPE_CALLBACK - Callback function pointer */
typedef void (*my_callback_t) GTY((callback))
  (struct my_struct *arg1, my_scalar_t arg2);
