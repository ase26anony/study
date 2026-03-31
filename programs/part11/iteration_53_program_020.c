/* Test types for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/opaque type */
struct opaque_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  float b;
  char c;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  int user_data;
};

/* TYPE_UNION */
union my_union {
  int i;
  float f;
  char *str;
};

/* TYPE_POINTER */
struct with_pointers {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void *void_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
  int arr[10];
  char str_arr[20];
  struct my_struct struct_arr[5];
};

/* TYPE_SCALAR */
typedef int my_scalar;
typedef unsigned long size_type;

/* TYPE_STRING */
struct with_strings {
  char *name;
  const char *const_name;
  char buffer[256];
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

struct with_callbacks {
  callback_func handler;
  simple_callback cleanup;
};

/* Nested structures for complex testing */
struct container {
  struct my_struct nested;
  union my_union choice;
  struct with_pointers *ptr_container;
};
