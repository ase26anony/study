/* Test types covering all TYPE_* enum values for gengtype */

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
  int fixed_array[10];
  char string_array[50];
  struct my_struct struct_array[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%0"))) lang_struct {
  int lang_specific;
  void *tree_node;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long scalar_type;

/* TYPE_STRING */
struct with_strings {
  char *dynamic_string;
  const char *const_string;
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, void*);
typedef int (*compare_func)(const void*, const void*);

struct with_callbacks {
  callback_func cb;
  compare_func cmp;
};

/* Composite type using multiple kinds */
struct composite {
  struct my_struct nested_struct;
  union my_union nested_union;
  int *pointer_member;
  int array_member[20];
  char *string_member;
  callback_func callback_member;
  my_scalar scalar_member;
};
