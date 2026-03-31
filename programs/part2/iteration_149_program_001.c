/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
  enum color color;
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
  struct tree_list *GTY((skip)) next;  /* Skip pointer for manual management */
  struct tree_list *GTY((chain_next ("next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *GTY((child)) children;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list ** GTY((length("list_count"))) list_array;
  size_t list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) base_binfo {
  tree base;
  tree binfos;
};

/* TYPE_STRING: String fields with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree decl;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void *GTY((skip)) user_data;
};

/* Composite structure using all types */
struct GTY(()) composite_type {
  /* Scalar fields */
  my_scalar count;
  my_long_scalar big_count;
  enum color current_color;
  
  /* String field */
  const char * GTY((tag("STRING"))) label;
  
  /* Pointer fields */
  struct plain_struct *plain_ptr;
  struct tree_list *list_head;
  struct recursive_node *root_node;
  
  /* Array field */
  struct array_container arrays;
  
  /* Union field */
  union value_union current_value;
  
  /* Language-specific structure */
  struct lang_specific_tree_node *lang_node;
  
  /* Callback field */
  struct tree_walker walker;
  
  /* Undefined type pointer (forward declared) */
  struct opaque_type *opaque_ptr;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  struct GTY((tag("TS_WITH_LANG_SPECIFIC"))) inner_lang_struct {
    tree node;
    int inner_data;
  } lang_part;
  
  struct GTY(()) inner_plain_struct {
    int x, y;
    tree GTY((skip)) skipped_tree;
  } plain_part;
  
  union GTY(()) inner_union {
    int as_int;
    tree as_tree;
    struct plain_struct *as_struct;
  } data_union;
};

/* Pointer to array of pointers */
struct GTY(()) pointer_network {
  struct tree_list *** GTY((length("network_size"))) complex_network;
  int network_size;
  struct recursive_node * GTY((skip)) *node_array;
  int node_count;
};

#endif /* TEST_COVERAGE_H */
