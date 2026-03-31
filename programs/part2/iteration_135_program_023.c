/* test-gtypes.h - Comprehensive test file for gengtype coverage */
#ifndef TEST_GTYPES_H
#define TEST_GTYPES_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef enum test_enum { ENUM_A, ENUM_B, ENUM_C } test_enum_t;
typedef bool test_bool_t;

/* TYPE_STRING: String type */
typedef const char *test_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*test_callback_fn)(void *data);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT: Standard C struct */
struct GTY(()) test_struct {
  int GTY((skip)) scalar_field;      /* TYPE_SCALAR */
  const char *GTY((tag("0"))) name;  /* TYPE_STRING */
  struct test_struct *GTY((skip)) next;  /* TYPE_POINTER */
  test_callback_fn GTY((skip)) callback; /* TYPE_CALLBACK */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
typedef struct GTY((user)) user_struct {
  int id;
  void *GTY((skip)) user_data;
} user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) test_union {
  int GTY((tag("0"))) int_val;
  double GTY((tag("1"))) double_val;
  struct test_struct *GTY((tag("2"))) struct_ptr;
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_container {
  int GTY((length("10"))) fixed_array[10];  /* Fixed-size array */
  struct test_struct *GTY((length("len"))) variable_array; /* Variable array */
  size_t len;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) pointer_test {
  struct test_struct *GTY((skip)) direct_ptr;
  struct opaque_struct *GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED target */
  void *GTY((skip)) generic_ptr;
  test_string_t GTY((skip)) string_ptr;  /* TYPE_STRING pointer */
};

/* Recursive structure for deep processing */
struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct *GTY((skip)) left;
  struct recursive_struct *GTY((skip)) right;
  union test_union GTY((skip)) data;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  struct array_container GTY((skip)) arrays;
  struct pointer_test GTY((skip)) pointers;
  test_enum_t GTY((skip)) enum_field;
  test_bool_t GTY((skip)) bool_field;
};

#endif /* TEST_GTYPES_H */
