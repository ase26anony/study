/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype during GCC build */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for pointer types */
struct GTY(()) plain_struct;
struct GTY(()) array_container;
struct GTY(()) tree_walker;

/* TYPE_UNDEFINED: Forward declaration of opaque type never defined */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Scalar typedef and enum */
typedef long my_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  color_enum color;
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
  tree t;
};

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct plain_struct *related;
  struct array_container *container;
  void *GTY((skip)) raw_ptr;  /* Skip raw void pointer */
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *children[4];
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  int list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) base_binfo {
  tree base;
  int offset;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("IDENTIFIER"))) identifier;
  tree decl;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void *GTY((skip)) user_data;
  tree root;
};

/* Structure containing all types */
struct GTY(()) master_container {
  /* TYPE_SCALAR fields */
  my_scalar count;
  color_enum current_color;
  
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_UNION */
  union value_union current_value;
  
  /* TYPE_POINTER */
  struct tree_list *head;
  struct recursive_node *root_node;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  struct base_binfo *binfo;
  
  /* TYPE_STRING */
  struct named_object named;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user_data;
  
  /* TYPE_UNDEFINED (pointer to forward-declared type) */
  struct opaque_type *GTY((skip)) opaque_ptr;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  int outer_id;
  struct GTY(()) inner_struct {
    int inner_id;
    tree value;
  } inner;
  struct inner_struct *inner_ptr;
};

/* Union within struct */
struct GTY(()) union_container {
  int tag;
  union {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((desc("tag"))) data;
};

#endif /* TEST_COVERAGE_H */
