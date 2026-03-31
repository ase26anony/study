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
  int int_array[10];
  char char_array[20];
  struct my_struct struct_array[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%0"))) lang_struct {
  int code;
  union my_union *values;
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long scalar_type;

/* TYPE_STRING */
struct with_strings {
  char *name;
  const char *const_name;
  char buffer[256];
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, char*);
typedef int (*compare_func)(const void*, const void*);

struct with_callbacks {
  callback_func cb;
  compare_func cmp;
};

/* Composite type using all kinds */
struct GTY(()) master_type {
  struct my_struct regular;
  struct GTY((user)) user_struct user;
  union my_union data;
  struct with_pointers *ptr_container;
  struct with_arrays arrays;
  struct lang_struct lang;
  my_scalar scalar;
  struct with_strings strings;
  struct with_callbacks callbacks;
  struct opaque_struct *opaque;  /* TYPE_UNDEFINED reference */
};
