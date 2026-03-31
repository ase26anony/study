/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char c;
};

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int int_arr[10];
  char char_arr[20];
  struct my_struct struct_arr[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%1"), tag("TREE"), chain_next("%h.next"))) tree_node {
  int code;
  struct tree_node *next;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING */
struct with_strings {
  char *str;
  const char *const_str;
  char name[32];
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
struct with_callbacks {
  callback_func handler;
  void (*void_handler)(void);
};

/* Complex nested structure to ensure all types are processed */
struct GTY(()) container {
  struct my_struct s;
  union my_union u;
  struct with_pointers *p;
  struct with_arrays a;
  struct tree_node *tree;
  my_scalar scalar;
  struct with_strings strings;
  struct with_callbacks callbacks;
};
