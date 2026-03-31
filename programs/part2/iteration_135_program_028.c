/* test-main.gtype - Main test file with comprehensive type definitions */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_STRUCT - Standard C struct */
struct GTY(()) my_struct {
  int a;
  float b;
  struct my_struct *next;  /* TYPE_POINTER */
  const char *name;        /* TYPE_STRING */
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int id;
  void *data;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int int_val;
  float float_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY - Fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];           /* Fixed array */
  struct my_struct *ptr_array[5]; /* Array of pointers */
  int GTY((length("len"))) *var_array; /* Variable length array */
  size_t len;
};

/* TYPE_SCALAR - Enumeration type */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_CALLBACK - Function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Struct containing callback */
struct GTY(()) callback_container {
  callback_func handler;
  void *user_data;
};

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *left;   /* TYPE_POINTER */
  struct tree_node *right;  /* TYPE_POINTER */
  enum color node_color;    /* TYPE_SCALAR */
};

/* Union containing array */
union GTY(()) union_with_array {
  int numbers[8];
  char chars[32];
  struct tree_node nodes[4];
};

/* Include language-specific structures */
#include "cp/test-cpp.gtype"
