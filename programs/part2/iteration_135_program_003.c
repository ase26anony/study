/* test-gty.h - Comprehensive GTY test types for gengtype coverage */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_STRUCT - Standard C struct */
struct GTY(()) my_struct {
  int a;
  double b;
  struct my_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT - User-defined struct with special handling */
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

/* TYPE_SCALAR - Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

/* TYPE_CALLBACK - Function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* TYPE_ARRAY - Fixed-size array in a struct */
struct GTY(()) array_container {
  int fixed_array[10];  /* TYPE_ARRAY of TYPE_SCALAR */
  int *dynamic_array GTY((length("len")));  /* Variable-length array */
  size_t len;
};

/* TYPE_STRING - Special string type */
typedef const char *GTY(()) gcc_string;

#endif /* TEST_GTY_H */
