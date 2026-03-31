/* test_types.gt - Comprehensive type definitions for gengtype coverage */

/* TYPE_UNDEFINED - incomplete/forward declaration */
struct undefined_struct;

/* TYPE_STRUCT - regular struct */
struct my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT - user-defined struct with GTY marker */
struct GTY((user)) user_struct {
  void *data;
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
  int fixed_arr[10];
  char str_arr[20];
  struct my_struct struct_arr[5];
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
  char filename[256];
};

/* TYPE_CALLBACK */
typedef void (*callback_func)(int, void*);
typedef int (*compare_func)(const void*, const void*);

struct with_callbacks {
  callback_func cb;
  compare_func cmp;
};

/* Complex nested structure to ensure all types are visited */
struct GTY(()) container {
  struct my_struct regular;
  struct GTY((user)) user_struct user;
  union my_union uni;
  struct with_pointers ptrs;
  struct with_arrays arrs;
  struct GTY((desc("%0"))) lang_struct lang;
  my_scalar scalar;
  struct with_strings strings;
  struct with_callbacks callbacks;
  struct undefined_struct *forward_ptr;  /* TYPE_UNDEFINED reference */
};

/* Enum type (treated as scalar) */
enum my_enum {
  ENUM_A,
  ENUM_B,
  ENUM_C
};

/* Function pointer arrays */
callback_func callback_array[5];

/* Nested pointer to array */
int (*pointer_to_array)[10];

/* Flexible array member */
struct flex_array {
  int count;
  int data[];
};
