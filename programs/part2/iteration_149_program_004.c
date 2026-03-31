/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to exercise all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* This will be TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  struct plain_struct *GTY((skip)) next;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: C union within GTY-marked struct */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

struct GTY(()) union_container {
  int tag;
  union value_union GTY((desc("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *prev;  /* Regular pointer */
  struct opaque_type *GTY((skip)) opaque_ref;  /* Pointer to undefined type */
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *parent;
  struct recursive_node *children[4];
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list ** GTY((length("list_count"))) list_array;
  int list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure pattern */
struct GTY((tag("TS_BASE"))) lang_base_node {
  tree base_type;
  struct lang_specific_tree_node *extended;
};

/* TYPE_STRING: String fields with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) filename;
  int line_number;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void *GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) complex_type {
  /* Scalar fields */
  my_scalar count;
  color_enum color;
  
  /* String field */
  const char * GTY((tag("STRING"))) description;
  
  /* Pointer fields */
  struct plain_struct *plain;
  struct user_defined *user;
  
  /* Array field */
  struct tree_list * GTY((length("list_len"))) items;
  int list_len;
  
  /* Union field */
  union value_union current_value;
  
  /* Callback field */
  walk_fn GTY((skip)) visitor;
  
  /* Nested language-specific structure */
  struct lang_specific_tree_node *lang_node;
};

/* Container with pointer to undefined type */
struct GTY(()) undefined_container {
  struct opaque_type *GTY((skip)) opaque_ptr;
  int defined_field;
};

#endif /* TEST_COVERAGE_H */
