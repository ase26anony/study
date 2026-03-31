/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;
  my_scalar count;
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

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;
  struct tree_list *prev;
  struct plain_struct *related;
};

struct GTY(()) pointer_network {
  struct tree_list *GTY((chain_next("next"))) head;
  struct pointer_network *self_ref;
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Forward declared type */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_buffer;
  int str_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_COMMON"))) common_tree_node {
  tree chain;
  tree type;
  enum tree_code code : 16;
};

/* TYPE_STRING: String type with tag attribute */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("IDENTIFIER"))) identifier;
};

/* More complex structure combining multiple types */
struct GTY(()) complex_type {
  /* TYPE_STRUCT elements */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union value;
  
  /* TYPE_POINTER network */
  struct tree_list *items;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific_tree_node *lang_node;
  
  /* TYPE_STRING */
  struct named_object naming;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  my_long_scalar long_scalar;
  color_enum color;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user_data;
};

/* Another union type for additional coverage */
union GTY(()) another_union {
  struct plain_struct *ps;
  struct array_container *ac;
  struct complex_type *ct;
};

/* Root structure containing everything */
struct GTY(()) root_container {
  struct complex_type main;
  union another_union alt;
  struct pointer_network *network;
  struct GTY((skip)) root_container *parent;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
