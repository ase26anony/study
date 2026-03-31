/* test-all-types.gtype */
#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration */
struct undefined_struct;

/* TYPE_STRUCT - standard struct */
struct GTY(()) my_struct {
  int a;
  double b;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - with user options */
typedef struct GTY((user)) user_struct {
  int id;
  const char *name;  /* TYPE_STRING */
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int int_val;
  double double_val;
  struct my_struct *struct_ptr;
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  struct my_struct *ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_SCALAR - enum type */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_CALLBACK - function pointer */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_type {
  struct my_struct base;
  union my_union data;
  enum color color;
  callback_func callback;
  struct complex_type *GTY((skip)) sibling;  /* TYPE_POINTER with skip */
  const char *GTY((tag("0"))) description;   /* TYPE_STRING with tag */
};

/* Variable length array using length option */
struct GTY(()) varray_struct {
  int length;
  struct my_struct * GTY((length ("%h.length"))) items[];
};

/* Chain structure for recursion */
struct GTY(()) chain_node {
  int value;
  struct chain_node *next;  /* Recursive pointer */
  struct chain_node *prev;  /* Another pointer */
};

/* Union with nested array */
union GTY(()) nested_union {
  int matrix[3][3];  /* Multi-dimensional array */
  struct chain_node *node_list[10];
};
