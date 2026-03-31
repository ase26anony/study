/* Test types for gengtype coverage - covers all TYPE_* kinds */

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
  char *str;
};

/* TYPE_POINTER - struct with pointer member */
struct with_ptr {
  int *int_ptr;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY - struct with array member */
struct with_array {
  int arr[10];
  char *str_array[5];
};

/* TYPE_LANG_STRUCT - simulate GCC internal type */
struct GTY((desc("%1"))) lang_struct {
  int code;
  union {
    int ival;
    double dval;
  } GTY((desc("%0.code"))) u;
};

/* TYPE_SCALAR - typedef of fundamental type */
typedef int my_scalar;
typedef double my_double;

/* TYPE_STRING - char* for strings */
struct with_string {
  char *name;
  const char *const_name;
};

/* TYPE_CALLBACK - function pointer */
typedef void (*callback_func)(int, void*);
struct with_callback {
  callback_func handler;
  void (*simple_handler)(void);
};

/* TYPE_NONE should never appear in valid input */

/* Additional complex types to ensure processing */
struct complex_type {
  struct my_struct nested;
  union my_union choice;
  struct with_ptr *next;
  callback_func callback;
  char *description;
  int values[8];
};

/* Root variable for scanning */
GTY(()) struct complex_type *root_var;
