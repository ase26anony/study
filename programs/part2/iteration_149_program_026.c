/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to exercise all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration of opaque type that is never defined */
struct GTY(()) opaque_type;
/* This should trigger write_state_undefined_type() */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;

enum color { RED, GREEN, BLUE };

/* Struct containing scalar types */
struct GTY(()) has_scalar {
  my_scalar count;
  my_long_scalar size;
  enum color color;
  unsigned int flags;
};

/* ==================== TYPE_STRUCT ==================== */
/* Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* tree is a known GCC type */
  struct has_scalar * GTY((tag("ptr_to_scalar"))) scalar_ptr;
};

/* Another struct for pointer network */
struct GTY(()) another_struct {
  const char *name;
  int value;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  const char * GTY((tag("STRING"))) s;
  double d;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_val;
    tree GTY((tag("TREE"))) tree_val;
    const char * GTY((tag("STRING"))) str_val;
  } GTY((desc("type"))) data;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer network with self-referential pointers */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip this pointer */
  struct tree_list * GTY((tag("nested_ptr"))) prev;
};

/* Pointer to undefined type */
struct GTY(()) uses_opaque {
  struct opaque_type * GTY((tag("opaque_ptr"))) opaque;
  struct plain_struct *normal_ptr;
};

/* Multiple pointer layers */
struct GTY(()) pointer_network {
  struct tree_list * GTY((tag("list_head"))) head;
  struct another_struct ** GTY((tag("ptr_ptr"))) ptr_array;
  void * GTY((skip)) raw_pointer;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Pointer to array with nested length */
  struct another_struct ** GTY((length("ptr_count"))) ptr_array;
  
  /* Scalar fields for lengths */
  int dynamic_count;
  unsigned int ptr_count;
  
  /* Two-dimensional array example */
  int matrix GTY((length("rows * cols")))[10];
  int rows;
  int cols;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned lang_flag1 : 1;
  unsigned lang_flag2 : 2;
  struct lang_specific_tree_node * GTY((skip)) lang_chain;
};

/* Another language struct with different tag */
struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  unsigned lexical_depth;
};

/* ==================== TYPE_STRING ==================== */
/* Structures with string fields */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  int id;
};

/* String in union context */
struct GTY(()) string_container {
  int string_type;
  union {
    const char * GTY((tag("STRING"))) str;
    int num;
  } GTY((desc("string_type"))) value;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

/* Struct with callback fields */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare_callback;
  void * GTY((skip)) user_data;
};

/* More complex callback example */
struct GTY(()) traversal_info {
  struct tree_walker *walker;
  int depth;
  tree current;
};

/* ==================== COMPLEX TYPE COMBINATIONS ==================== */
/* Struct that combines many types to ensure all code paths are exercised */
struct GTY(()) master_container {
  /* TYPE_STRUCT members */
  struct plain_struct plain;
  struct another_struct another;
  
  /* TYPE_UNION */
  union value_union union_val;
  
  /* TYPE_POINTER network */
  struct tree_list *list;
  struct pointer_network *network;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  
  /* TYPE_STRING */
  struct named_object named;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  enum color color_field;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user_data;
  
  /* TYPE_UNDEFINED reference */
  struct opaque_type * GTY((tag("undefined_ref"))) undefined_ref;
};

/* Root structure for gengtype to process */
extern struct GTY(()) root_struct {
  struct master_container *main;
  struct tree_list *global_list;
  struct named_object * GTY((length("object_count"))) objects;
  int object_count;
} global_root;

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
