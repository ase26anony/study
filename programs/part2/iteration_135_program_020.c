/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_undefined;

/* TYPE_STRUCT: Standard C struct */
struct GTY(()) my_struct {
  int a;
  double b;
};

/* TYPE_USER_STRUCT: User-defined struct with custom options */
typedef struct GTY((user)) user_struct {
  int id;
  const char *name;
} user_struct_t;

/* TYPE_UNION: C union type */
union GTY(()) my_union {
  int ival;
  double dval;
  void *pval;
};

/* TYPE_POINTER: Pointer to another GTY-tagged type */
struct GTY(()) linked_node {
  int value;
  struct linked_node *GTY((skip)) next;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Fixed-size array */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  int *GTY((length("len"))) variable_array;  /* Variable-length array */
  size_t len;
};

/* TYPE_SCALAR: Basic scalar types */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_STRING: String type */
struct GTY(()) string_container {
  const char *GTY((tag("0"))) filename;  /* TYPE_STRING */
  const char *message;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY((callback)) callback_func)(void *data);

struct GTY(()) callback_container {
  callback_func handler;  /* TYPE_CALLBACK */
  void *user_data;
};

/* Recursive structure to ensure deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((skip)) left;   /* TYPE_POINTER */
  struct tree_node *GTY((skip)) right;  /* TYPE_POINTER */
  union GTY(()) data {                  /* TYPE_UNION containing TYPE_ARRAY */
    int numbers[5];
    double values[3];
  } data_union;
};

/* Complex nested example */
struct GTY(()) complex_type {
  struct my_struct base;          /* TYPE_STRUCT */
  user_struct_t user;             /* TYPE_USER_STRUCT */
  union myUnion variant;          /* TYPE_UNION */
  enum color color;               /* TYPE_SCALAR (enum) */
  const char *description;        /* TYPE_STRING */
  callback_func notify;           /* TYPE_CALLBACK */
  struct array_container arrays;  /* TYPE_STRUCT containing TYPE_ARRAY */
};
