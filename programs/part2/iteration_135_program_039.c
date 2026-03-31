/* test-gtypes.h - Comprehensive test of all gengtype type classifications */

#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED - Forward declaration without definition */
struct undefined_struct;

/* TYPE_STRUCT - Standard struct */
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
  int int_val;
  double double_val;
  void *ptr_val;  /* TYPE_POINTER */
};

/* TYPE_SCALAR - Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE
} color_t;

/* TYPE_CALLBACK - Function pointer type */
typedef void GTY((callback)) (*callback_func)(int, const char*);

/* TYPE_ARRAY - Fixed size array within a struct */
struct GTY(()) array_container {
  int fixed_array[10];  /* Fixed-size array */
  int *dynamic_array;   /* Pointer for variable-length array */
  size_t length;
};

/* TYPE_STRING - String pointer type */
typedef const char * GTY(()) string_ptr;

#endif /* TEST_GTYPES_H */
