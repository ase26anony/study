/* test-coverage.h - Header file to test gengtype state generation coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For 'tree' type used in examples */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
  struct plain_struct *next;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* Struct containing a union */
struct GTY(()) union_container {
  int type;
  union value_union GTY((desc("type"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for manual management */
  struct tree_list **GTY((skip)) prev_ptr;  /* Pointer to pointer */
};

/* Self-referential structure */
struct GTY(()) linked_node {
  int id;
  struct linked_node *self_ptr;  /* Self-reference */
  struct linked_node *children[4];  /* Array of pointers */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Another variable-length array using field reference */
  struct plain_struct ** GTY((length("item_count"))) items;
  
  int dynamic_count;
  size_t item_count;
  
  /* Nested array in a pointer */
  int (*matrix)[10] GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific pattern */
struct GTY((tag("LANG_TYPE"))) c_type {
  tree main_variant;
  tree context;
};

/* TYPE_SCALAR: Scalar types and typedefs */
typedef int my_scalar;
typedef unsigned long hashval_t;

enum gty_test_enum {
  GTY_TEST_A,
  GTY_TEST_B,
  GTY_TEST_C
};

struct GTY(()) has_scalars {
  my_scalar count;
  hashval_t hash;
  enum gty_test_enum kind;
  float float_val;
  double double_val;
  _Bool flag;
};

/* TYPE_STRING: String fields */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  char * GTY((tag("STRING"))) mutable_str;
};

/* TYPE_CALLBACK: Function pointers */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) master_container {
  /* Nested structures */
  struct plain_struct plain GTY((skip));
  
  /* Pointer to user-defined type */
  struct user_defined * GTY((skip)) user_data;
  
  /* Union field */
  union value_union current_value;
  
  /* Array of pointers */
  struct tree_list * GTY((length("list_count"))) lists;
  int list_count;
  
  /* String */
  const char * GTY((tag("STRING"))) container_name;
  
  /* Callback */
  walk_fn GTY((skip)) iterate;
  
  /* Scalar */
  enum gty_test_enum container_type;
};

/* Additional pointer types for coverage */
typedef struct plain_struct *plain_ptr;
typedef const struct plain_struct *const_plain_ptr;

/* Array of function pointers */
typedef void (*action_fn[5])(void);

struct GTY(()) callback_array {
  action_fn GTY((skip)) actions;
};

#endif /* TEST_COVERAGE_H */
