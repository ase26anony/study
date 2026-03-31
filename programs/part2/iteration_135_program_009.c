/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_STRUCT - standard struct with GTY marker */
struct GTY(()) my_struct {
  int a;                    /* TYPE_SCALAR */
  const char *name;         /* TYPE_STRING */
  struct my_struct *next;   /* TYPE_POINTER */
  int arr[10];              /* TYPE_ARRAY */
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct GTY((user)) user_struct {
  int id;
  void *data;
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int i;
  float f;
  struct my_struct *s;
};

/* TYPE_LANG_STRUCT - language-specific structure */
/* This will be in a separate C++ file for proper handling */

/* TYPE_CALLBACK - function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_struct {
  struct my_struct base;
  union my_union variant;
  callback_func handler;
  struct complex_struct *GTY((skip)) children[5];  /* Array of pointers */
  int *GTY((length("len"))) dynamic_array;
  size_t len;
};

/* Linked list example for recursive processing */
struct GTY(()) linked_list {
  int value;
  struct linked_list *next;
  struct linked_list *prev;
};

/* Tree structure example */
struct GTY(()) tree_node {
  int data;
  struct tree_node *left;
  struct tree_node *right;
};

/* Enum type (also TYPE_SCALAR) */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* Structure containing enum */
struct GTY(()) color_container {
  enum color bg_color;
  enum color fg_color;
};

/* Variable length array using length option */
struct GTY(()) var_array_struct {
  int count;
  int GTY((length("count"))) items[];
};

/* Nested union within struct */
struct GTY(()) nested_union_container {
  int type;
  union {
    int int_val;
    float float_val;
    const char *string_val;
  } GTY((desc("type"))) value;
};
