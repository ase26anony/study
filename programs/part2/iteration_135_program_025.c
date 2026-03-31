/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_STRUCT - standard struct */
struct GTY(()) my_struct {
  int a;
  double b;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct GTY((user)) user_struct {
  int id;
  const char *name;  /* TYPE_STRING */
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;  /* TYPE_POINTER */
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  struct my_struct *variable_array GTY((length("%0.count")));  /* Variable length array */
  int count;
};

/* TYPE_SCALAR - enum type */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_STRING - string pointer type */
struct GTY(()) string_container {
  const char *filename;  /* TYPE_STRING */
  const char *message;
};

/* TYPE_CALLBACK - function pointer */
typedef void (*GTY(()) callback_func)(int, const char*);

struct GTY(()) callback_container {
  callback_func handler;  /* TYPE_CALLBACK */
  void *user_data;
};

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *left;   /* TYPE_POINTER */
  struct tree_node *right;  /* TYPE_POINTER */
  union myunion *data;      /* TYPE_POINTER to TYPE_UNION */
};

/* Complex nested structure */
struct GTY(()) complex_type {
  struct my_struct base;
  union my_union variant;
  struct array_container arrays;
  enum color color;
  struct string_container strings;
  struct callback_container callbacks;
  struct tree_node *tree_root;
};
