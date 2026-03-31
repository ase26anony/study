/* test-all-types.gtype - Comprehensive test for all gengtype type kinds */

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - forward declaration of opaque struct */
struct opaque_undefined;

/* TYPE_STRUCT - standard C struct */
struct GTY(()) my_struct {
  int a;
  double b;
  const char *name;
};

/* TYPE_USER_STRUCT - struct with user-defined options */
typedef struct GTY((user)) user_struct {
  int id;
  void * GTY((skip)) user_data;  /* skipped field */
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int int_val;
  double double_val;
  char * GTY((length("strlen($)"))) string_val;
};

/* TYPE_POINTER - self-referential pointer for linked list */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((tag("0"))) next;
};

/* TYPE_ARRAY - various array types */
struct GTY(()) array_container {
  int fixed_array[10];
  int * GTY((length("len"))) variable_array;
  size_t len;
};

/* TYPE_SCALAR - enum type */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_STRING */
struct GTY(()) string_container {
  const char * GTY((tag("0"))) filename;
  const char *message;
};

/* TYPE_CALLBACK - function pointer */
typedef void (* GTY(()) callback_func)(int, void*);

struct GTY(()) callback_container {
  callback_func handler;
  void *context;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_type {
  struct my_struct base;          /* TYPE_STRUCT */
  union myUnion variant;          /* TYPE_UNION */
  struct linked_node *list;       /* TYPE_POINTER */
  int matrix[5][5];              /* TYPE_ARRAY (2D) */
  enum color current_color;       /* TYPE_SCALAR */
  const char *description;        /* TYPE_STRING */
  callback_func on_event;         /* TYPE_CALLBACK */
};

/* Variable-length array with pointer */
struct GTY(()) var_len_struct {
  int count;
  struct my_struct * GTY((length("count"))) items;
};
