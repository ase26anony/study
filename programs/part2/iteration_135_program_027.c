/* Test file for gengtype coverage - covers TYPE_STRUCT, TYPE_UNION, TYPE_POINTER, etc. */

#include "system.h"
#include "coretypes.h"
#include "tm.h"

/* TYPE_UNDEFINED - forward declaration */
struct opaque_struct;

/* TYPE_STRUCT - basic struct */
struct GTY(()) my_struct {
  int a;
  double b;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT */
typedef struct GTY((user)) user_struct {
  int id;
  const char *name;  /* TYPE_STRING */
} user_struct_t;

/* TYPE_UNION */
union GTY(()) my_union {
  int ival;
  double dval;
  struct my_struct *sptr;  /* TYPE_POINTER inside union */
};

/* TYPE_ARRAY - fixed size array */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  struct my_struct *ptr_array[5];  /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_SCALAR - enum definition */
enum GTY(()) color {
  RED,
  GREEN,
  BLUE
};

/* TYPE_CALLBACK - function pointer */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* Recursive structure for deep processing */
struct GTY(()) tree_node {
  int value;
  struct tree_node *GTY((skip)) left;  /* TYPE_POINTER with skip option */
  struct tree_node *right;  /* TYPE_POINTER */
  union my_union data;  /* TYPE_UNION field */
};

/* Container with multiple type kinds */
struct GTY(()) type_container {
  struct my_struct basic_struct;  /* TYPE_STRUCT field */
  union my_union basic_union;     /* TYPE_UNION field */
  enum color color_enum;          /* TYPE_SCALAR field */
  const char *filename;           /* TYPE_STRING field */
  callback_func callback;         /* TYPE_CALLBACK field */
  int scalar_int;                 /* TYPE_SCALAR field */
  struct opaque_struct *opaque;   /* TYPE_POINTER to TYPE_UNDEFINED */
};
