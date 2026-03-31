/* test-base.gtype - Base type definitions covering most switch cases */

/* TYPE_SCALAR - Fundamental scalar types */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());

/* TYPE_STRING - String type */
typedef const char *my_string_t GTY(());

/* TYPE_UNDEFINED - Forward declaration to create undefined type */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr_t GTY(());

/* TYPE_STRUCT - Basic structure with various fields */
struct my_struct GTY(())
{
  my_scalar_t field1;
  my_ulong_t field2;
  my_string_t field3 GTY((skip));  /* Skip from GC */
  struct my_struct *next GTY((chain_next));  /* Linked list */
};

/* TYPE_UNION - Discriminated union */
union my_union GTY((desc("tag")))
{
  int tag;
  struct {
    my_scalar_t int_val;
  } GTY((tag("0"))) as_int;
  struct {
    my_string_t str_val;
  } GTY((tag("1"))) as_str;
  struct {
    struct my_struct *ptr_val;
  } GTY((tag("2"))) as_ptr;
};

/* TYPE_ARRAY - Both fixed and variable length arrays */
struct array_container GTY(())
{
  int length;
  struct my_struct *variable_array GTY((length("length")));
  my_scalar_t fixed_array[10];
  union my union *union_array GTY((length("5")));
};

/* TYPE_POINTER - Various pointer types */
typedef struct my_struct *struct_ptr_t GTY(());
typedef union my_union *union_ptr_t GTY(());
typedef my_scalar_t *scalar_ptr_t GTY(());
typedef struct array_container *array_ptr_t GTY(());

/* Nested structure with complex relationships */
struct complex_struct GTY(())
{
  struct_ptr_t struct_ptr;
  union_ptr_t union_ptr;
  scalar_ptr_t scalar_ptr;
  array_ptr_t array_ptr;
  struct complex_struct *prev GTY((chain_prev("next")));
  struct complex_struct *next GTY((chain_next));
};
