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

/* TYPE_SCALAR: Basic scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
  my_scalar count;
  enum color col;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* Struct containing a union */
struct GTY(()) union_container {
  int tag;
  union GTY((desc("tag"))) {
    int as_int;
    const char *as_string;
  } GTY((tag("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip for manual management */
  struct tree_list *prev;
};

/* Self-referential structure */
struct GTY(()) linked_node {
  int data;
  struct linked_node *GTY((skip)) next;
  struct linked_node *child;
};

/* Pointer to opaque type */
struct GTY(()) uses_opaque {
  struct opaque_type *GTY((skip)) opaque_ptr;
  int valid;
};

/* TYPE_ARRAY: Arrays with different GTY annotations */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Another variable-length array using field reference */
  struct plain_struct * GTY((length("item_count"))) struct_array;
  
  int dynamic_count;
  int item_count;
  
  /* Nested array in a struct */
  struct {
    char buffer[100];
  } nested;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimics tree structure with language-specific node */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((skip)) decl;
  int lang_specific_flags;
};

/* Another language-specific pattern */
struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  tree supercontext;
  int block_number;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  int id;
};

/* Structure with multiple string types */
struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) title;
  char * GTY((tag("STRING"))) mutable_str;
  struct named_object *obj;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
};

/* Structure with callback and other fields */
struct GTY(()) callback_container {
  int mode;
  walk_fn GTY((skip)) processor;
  struct tree_walker *walker;
};

/* Complex nested structure to test multiple type interactions */
struct GTY(()) master_container {
  /* Various type references */
  struct plain_struct plain;
  struct user_defined *user;
  union value_union data;
  struct tree_list *list;
  struct array_container arrays;
  struct lang_specific_tree_node *lang_node;
  struct named_object named;
  struct tree_walker walker;
  
  /* Additional pointers */
  struct master_container *GTY((skip)) next;
  struct master_container *child;
  
  /* Arrays of different types */
  struct plain_struct * GTY((length("plain_count"))) plain_array;
  tree * GTY((length("tree_count"))) tree_array;
  
  int plain_count;
  int tree_count;
};

/* Global variable declarations for gengtype to process */
extern struct master_container * GTY((root)) global_container;
extern struct tree_list * GTY((root)) global_tree_list;
extern struct array_container GTY((root)) global_array;

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
