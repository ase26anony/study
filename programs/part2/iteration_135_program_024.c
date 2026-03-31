/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

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
  struct my_struct *variable_array GTY((length("len")));  /* Variable length array */
  int len;
};

/* TYPE_SCALAR - enum type */
enum GTY(()) my_enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
};

/* TYPE_CALLBACK - function pointer */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Struct containing callback */
struct GTY(()) callback_container {
  callback_func func;
  int data;
};

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *left;   /* TYPE_POINTER */
  struct tree_node *right;  /* TYPE_POINTER */
  enum my_enum node_type;   /* TYPE_SCALAR */
};

/* Union with array */
union GTY(()) complex_union {
  int ints[5];              /* TYPE_ARRAY */
  struct my_struct data;    /* TYPE_STRUCT */
  user_struct_t *user_ptr;  /* TYPE_POINTER to TYPE_USER_STRUCT */
};
