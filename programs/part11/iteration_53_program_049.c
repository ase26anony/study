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
  void* data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char* s;
};

/* TYPE_POINTER */
struct with_pointers {
  int* int_ptr;
  struct my_struct* struct_ptr;
  void* void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int fixed_array[10];
  char string_array[20];
  struct my_struct struct_array[5];
};

/* TYPE_LANG_STRUCT - simulate GCC language-specific type */
struct GTY((tag("TREE"))) tree_node {
  int code;
  union tree_node* GTY((skip)) chain;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING */
struct with_strings {
  char* dynamic_string;
  const char* const_string;
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, char*);
typedef int (*compare_func)(const void*, const void*);

/* Nested structures to ensure full traversal */
struct container {
  struct my_struct nested_struct;
  union my_union nested_union;
  struct with_pointers* ptr_member;
  callback_func handler;
};

/* Forward declarations for pointer types */
struct forward_declared;
struct another_forward;

struct uses_forward {
  struct forward_declared* fwd_ptr;
  struct another_forward** double_ptr;
};
