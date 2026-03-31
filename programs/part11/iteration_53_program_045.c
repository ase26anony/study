/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar;

/* TYPE_STRING - string type */
typedef const char *my_string;

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char*);

/* TYPE_POINTER - pointer type */
struct pointer_container {
  int *int_ptr;
  struct opaque_struct *opaque_ptr;
};

/* TYPE_ARRAY - array type */
struct array_container {
  int fixed_array[10];
  int variable_array[];
};

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int id;
  my_scalar value;
  my_string name;
  callback_func callback;
  struct pointer_container *ptr_container;
  int array_member[5];
};

/* TYPE_UNION - union type */
union my_union {
  int int_val;
  float float_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
  void *user_pointer;
};

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
struct GTY((desc("%0"), tag("TREE_CODE"), chain_next("%h.next"))) tree_node {
  int code;
  union tree_node *next;
};

/* Container that uses all types */
struct all_types_container {
  struct my_struct regular_struct;
  union my_union the_union;
  struct user_struct *user_struct_ptr;
  struct tree_node *lang_struct_ptr;
  my_scalar scalar_value;
  my_string string_value;
  callback_func callback_value;
  int *pointer_to_int;
  int array_of_scalars[20];
};
