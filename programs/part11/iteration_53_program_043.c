/* Test types for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_STRUCT */
struct my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT - using GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char *s;
};

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int fixed_arr[10];
  char str_arr[5][20];
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING */
struct with_strings {
  char *name;
  const char *const_name;
  char filename[256];
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
struct with_callbacks {
  callback_func handler;
  void (*cleanup)(void*);
};

/* Nested complex type */
struct complex_type {
  struct my_struct nested_struct;
  union my_union nested_union;
  struct with_pointers *ptr_member;
  callback_func handlers[5];
};
