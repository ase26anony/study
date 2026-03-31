/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - triggers TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  enum color color;
};

/* TYPE_USER_STRUCT: User-defined type handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: C union within GTY-marked struct */
union GTY(()) value_union {
  int i;
  const char *s;
  tree t;
};

struct GTY(()) union_container {
  int tag;
  union value_union GTY((desc("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *GTY((chain_next("next"))) chain_next;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_list *list_ptr;
  struct pointer_network *self_ptr;  /* Self-referential */
  void *GTY((skip)) opaque_ptr;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_array;
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
  enum tree_code code : 8;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* More complex structure combining multiple types */
struct GTY(()) complex_type {
  /* TYPE_SCALAR fields */
  my_scalar count;
  my_long_scalar big_count;
  enum color current_color;
  
  /* TYPE_POINTER fields */
  struct plain_struct *struct_ptr;
  struct tree_list *list_head;
  
  /* TYPE_ARRAY field */
  struct array_container * GTY((length("container_count"))) containers;
  int container_count;
  
  /* TYPE_STRING field */
  const char * GTY((tag("STRING"))) type_name;
  
  /* TYPE_CALLBACK field */
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  
  /* TYPE_UNION field */
  union value_union current_value;
};

/* Another user structure for additional coverage */
struct GTY((user)) another_user_struct {
  int id;
  char *name;
};

/* Structure with nested anonymous union (tests edge cases) */
struct GTY(()) with_anonymous_union {
  int tag;
  union {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((desc("tag"))) data;
};

/* Array of pointers with length specifier */
struct GTY(()) pointer_array_container {
  tree * GTY((length("ptr_count"))) pointers;
  int ptr_count;
  struct plain_struct ** GTY((length("struct_count"))) structs;
  int struct_count;
};

/* Structure for testing param_is/param1_is attributes */
struct GTY(()) base_param {
  int base_data;
};

struct GTY(()) derived_param {
  struct base_param base;
  int extra_data;
};

struct GTY(()) param_container {
  struct base_param * GTY((param_is(struct derived_param))) param_ptr;
  int discriminator;
};

#endif /* TEST_COVERAGE_H */
