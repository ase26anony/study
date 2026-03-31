/* Test header for gengtype coverage - contains all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
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
  char *string_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((skip)) union_ptr_skip;
extern my_scalar_type * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
extern struct my_test_struct * GTY(()) struct_ptr_array[5];
extern const char * GTY(()) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char *);
typedef int (*GTY((length("len"))) array_callback)(int *data, size_t len);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), tag("TREE_TYPE"))) lang_tree_node {
  enum tree_code type;
  union {
    int ival;
    double dval;
    const char *sval;
  } GTY((desc("((enum tree_code)%0.type == INTEGER_CST) ? 1 : \
               ((enum tree_code)%0.type == REAL_CST) ? 2 : 3"))) u;
};

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains one of each type category */
  my_scalar_type scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) string_field;    /* TYPE_STRING */
  struct my_test_struct struct_field;   /* TYPE_STRUCT */
  union my_test_union union_field;      /* TYPE_UNION */
  struct my_test_struct *ptr_field;     /* TYPE_POINTER */
  int array_field[8];                   /* TYPE_ARRAY */
  callback_func callback_field;         /* TYPE_CALLBACK */
  struct lang_tree_node lang_field;     /* TYPE_LANG_STRUCT */
};

/* Variable declarations using our types */
extern struct complex_container GTY(()) global_container;
extern struct my_test_struct GTY(()) struct_array[4][2];

/* Parameterized pointer with chain_next */
struct GTY((chain_next("%h.next"))) chainable_struct {
  int value;
  struct chainable_struct *next;
};

/* Type with conditional fields */
struct GTY(()) conditional_struct {
  int type;
  union {
    int ival;
    double dval;
    struct my_test_struct *sptr;
  } GTY((desc("%0.type"))) data;
};

#endif /* MYTEST_GTY_H */
