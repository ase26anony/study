/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;
typedef struct opaque_struct undefined_type;

/* TYPE_SCALAR - fundamental type */
typedef int my_scalar;

/* TYPE_STRING - string type */
typedef const char *my_string;

/* TYPE_POINTER - pointer type */
typedef int *int_ptr;

/* TYPE_ARRAY - array type */
typedef int int_array[10];

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  float b;
  int_ptr c;
};

/* TYPE_UNION - union type */
union my_union {
  int i;
  float f;
  char *s;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, const char*);

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("tree_node"))) lang_struct {
  int code;
  void *chain;
};

/* Complex structure containing multiple types */
struct GTY(()) container {
  my_scalar scalar_field;
  my_string string_field;
  int_ptr pointer_field;
  int_array array_field;
  struct my_struct struct_field;
  union my_union union_field;
  callback_func callback_field;
  struct lang_struct *lang_field;
  struct user_struct *user_field;
  undefined_type *undefined_field;
};

/* Another structure for nested coverage */
struct GTY(()) nested_types {
  struct container *container_ptr;
  struct nested_types *next;
};
