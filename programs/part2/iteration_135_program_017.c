/* Test file to cover gengtype-state.cc switch cases */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration */
struct opaque_struct;

/* TYPE_STRUCT - standard struct */
struct GTY(()) my_struct {
  int a;
  const char *name;  /* TYPE_STRING */
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT */
typedef struct GTY((user)) user_struct {
  int id;
  void *data;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY */
struct GTY(()) array_container {
  int fixed_array[10];  /* Fixed-size array */
  int GTY((length("len"))) *var_array;  /* Variable-length array */
  size_t len;
};

/* TYPE_SCALAR - enum */
enum GTY(()) my_enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
};

/* TYPE_CALLBACK */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Struct containing callback */
struct GTY(()) callback_container {
  callback_func func;
  int priority;
};

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *left;
  struct tree_node *right;
  struct tree_node **children;  /* Array of pointers */
  size_t child_count;
};

/* Opaque pointer type */
typedef struct opaque_struct *GTY(()) opaque_ptr_t;
