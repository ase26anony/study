/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;        /* Using typedef scalar */
};

/* TYPE_USER_STRUCT: User-defined type handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
};

/* TYPE_UNION: C union within GTY context */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* Another union as a field within a struct */
struct GTY(()) union_container {
  int tag;
  union GTY((desc("tag"))) {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((tag("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Self-referential skip pointer */
  struct plain_struct *nested;         /* Pointer to another GTY struct */
};

/* More complex pointer structure */
struct GTY(()) pointer_network {
  struct tree_list *GTY((skip)) head;
  struct pointer_network **GTY((skip)) refs;  /* Pointer to pointer */
  int ref_count;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char ** GTY((length("name_count"))) names;
  int name_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking tree structure language tags */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BLOCK"))) lang_block {
  tree statements;
  tree declarations;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  int id;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) comparator;
  tree start_node;
};

/* Complex nested structure to test multiple type combinations */
struct GTY(()) complex_nested {
  struct plain_struct base;
  union value_union data;
  struct tree_list *items;
  struct array_container arrays;
  struct named_object name_info;
  struct tree_walker walker;
  enum color color;
};

/* Root structure that references everything */
struct GTY(()) root_container {
  struct plain_struct plain;
  struct user_defined *user;  /* Pointer to user struct */
  union value_union union_val;
  struct tree_list *list;
  struct pointer_network *network;
  struct array_container array_data;
  struct lang_specific_tree_node *lang_node;
  struct lang_block *block;
  struct named_object named;
  struct tree_walker walker;
  struct complex_nested *nested;
  
  /* Direct scalar and string fields */
  my_scalar scalar_field;
  my_long_scalar long_scalar;
  const char * GTY((tag("STRING"))) root_name;
  
  /* Array of various types */
  struct plain_struct * GTY((length("plain_count"))) plain_array;
  int plain_count;
  
  /* Callback array */
  walk_fn GTY((skip)) callbacks[3];
};

/* External declaration to force processing */
extern struct root_container GTY((root)) global_root;

#endif /* TEST_COVERAGE_H */
