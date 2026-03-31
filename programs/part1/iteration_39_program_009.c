/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_type field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(struct my_user_struct *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((skip)) union_ptr_skip;
extern my_scalar_type * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) fixed_array[10];
extern struct my_test_struct * GTY((length("len"))) variable_array[];
extern const char * GTY(()) string_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char *);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), tag("LANG_STRUCT"))) lang_specific {
  enum { LANG_INT, LANG_FLOAT, LANG_STRING } type;
  union {
    int int_val;
    double float_val;
    const char *string_val;
  } GTY((desc("(%1.type == LANG_STRING) ? 1 : 0"))) u;
};

/* Nested/complex types for thorough testing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_type scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) string_field;    /* TYPE_STRING */
  struct my_test_struct *struct_field;  /* TYPE_POINTER to TYPE_STRUCT */
  int GTY(()) array_field[20];          /* TYPE_ARRAY */
  callback_func callback_field;         /* TYPE_CALLBACK */
  union my_test_union union_field;      /* TYPE_UNION */
};

/* Template-like macro for generating multiple instances */
#define DECLARE_TEST_TYPE(name) \
  struct GTY(()) name##_struct { \
    int id; \
    struct name##_struct * GTY((skip)) next; \
  }; \
  extern struct name##_struct * GTY(()) name##_list;

DECLARE_TEST_TYPE(test1)
DECLARE_TEST_TYPE(test2)
DECLARE_TEST_TYPE(test3)

/* Extern declarations to satisfy the compiler */
extern int my_array_length;

#endif /* MYTEST_GTY_H */
