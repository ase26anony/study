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

/* TYPE_POINTER - struct with pointer member */
struct with_ptr {
  int *p;
  struct my_struct *next;
};

/* TYPE_ARRAY - struct with array member */
struct with_array {
  int arr[10];
  char name[32];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%0"))) lang_struct {
  int code;
  void *values;
};

/* TYPE_SCALAR - typedef of fundamental type */
typedef int my_scalar;
typedef unsigned long my_unsigned;

/* TYPE_STRING - char pointer for strings */
struct with_string {
  char *name;
  const char *path;
};

/* TYPE_CALLBACK - function pointer type */
typedef void (*callback_func)(int, void*);
struct with_callback {
  callback_func handler;
  void (*cleanup)(void);
};

/* TYPE_NONE should not appear in valid input, but gcc_unreachable handles it */

/* Composite structure using multiple types */
struct container {
  struct my_struct base;
  union my_union data;
  struct with_ptr *ptr_field;
  struct with_array array_field;
  my_scalar count;
  char *description;
  callback_func notify;
};
