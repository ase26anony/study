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

/* TYPE_STRING: String pointer */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_type field2;
  const char * GTY((skip)) name;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  struct my_base_struct * GTY((tag("0"))) base;
  void (* GTY((skip)) cleanup)(void);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_base_struct * GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct my_base_struct * GTY(()) global_struct_ptr;
union my_test_union * GTY((chain_next("next"))) union_ptr_chain;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
struct my_base_struct GTY(()) struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), chain_next("%0.next"))) lang_specific_struct {
  enum tree_code type;
  struct lang_specific_struct *next;
  union {
    int ival;
    double dval;
  } GTY((desc("TYPE_PRECISION(%0.type) > 32"))) u;
};

/* Nested structure for complex testing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_type scalar_field;          /* TYPE_SCALAR */
  const char * GTY((length("strlen(%0)+1"))) string_field; /* TYPE_STRING */
  struct my_base_struct struct_field;   /* TYPE_STRUCT */
  union my_test_union union_field;      /* TYPE_UNION */
  struct container_struct * GTY((skip)) next; /* TYPE_POINTER */
  callback_func callback_field;         /* TYPE_CALLBACK */
  struct lang_specific_struct lang_field; /* TYPE_LANG_STRUCT */
  
  /* Array fields */
  int GTY(()) matrix[3][3];             /* TYPE_ARRAY (multi-dimensional) */
  struct my_base_struct * GTY(()) ptr_array[4]; /* TYPE_ARRAY of TYPE_POINTER */
};

/* Variable-length array with length specifier */
struct GTY(()) varray_struct {
  int count;
  int GTY((length("%h.count"))) data[];
};

/* Union containing pointer for desc/tag testing */
union GTY((desc("0"))) tagged_union {
  int type;
  struct my_base_struct * GTY((tag("0"))) s;
  union my_test_union * GTY((tag("1"))) u;
};

#endif /* MYTEST_GTY_H */
