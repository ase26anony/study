/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in examples */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration of opaque type that is never defined */
struct GTY(()) opaque_type;

/* ==================== TYPE_STRUCT ==================== */
/* Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC marking */
  double value;
};

/* Another struct with nested structures */
struct GTY(()) outer_struct {
  struct plain_struct GTY((tag("0"))) inner;
  int count;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
  /* User must provide marking routines for this type */
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  double d;
  const char * GTY((tag("STRING"))) s;
  tree t;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int tag;
  union {
    int int_val;
    tree GTY((tag("1"))) tree_val;
    const char * GTY((tag("STRING"))) str_val;
  } GTY((desc("tag"))) data;
};

/* ==================== TYPE_POINTER ==================== */
/* Self-referential pointer structure (like linked list) */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip pointer for manual management */
  struct tree_list * GTY((tag("0"))) prev;
};

/* Complex pointer network */
struct GTY(()) pointer_network {
  struct plain_struct * GTY((tag("0"))) plain_ptr;
  struct tree_list * GTY((tag("0"))) list_ptr;
  void * GTY((skip)) opaque_ptr;  /* Skip this pointer */
  struct pointer_network * GTY((tag("0"))) self_ptr;  /* Self-reference */
};

/* Pointer to undefined type */
struct GTY(()) uses_opaque {
  struct opaque_type * GTY((tag("0"))) opaque_ptr;
  int valid;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("fixed_count"))) fixed_trees[3];
  int fixed_count;
};

/* Struct with variable-length array */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  struct plain_struct * GTY((length("dynamic_count"), tag("0"))) struct_array;
};

/* Multi-dimensional array example */
struct GTY(()) matrix_container {
  int rows;
  int cols;
  double * GTY((length("rows * cols"))) matrix;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure (mimicking tree structure) */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("1"))) decl;
  unsigned int lang_specific_flags;
};

/* Another language structure with different tag */
struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  tree supercontext;
  tree chain;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef */
typedef int my_scalar;
typedef unsigned long my_ulong;
typedef enum { RED, GREEN, BLUE } color_enum;

/* Struct with various scalar types */
struct GTY(()) has_scalars {
  my_scalar count;
  my_ulong size;
  color_enum color;
  float ratio;
  double precision;
  char small;
  unsigned char flags;
};

/* Enum as standalone type */
enum GTY(()) error_codes {
  ERR_NONE = 0,
  ERR_SYNTAX,
  ERR_SEMANTIC,
  ERR_RUNTIME
};

/* ==================== TYPE_STRING ==================== */
/* String type with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  int id;
};

/* Multiple string fields */
struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) title;
  const char * GTY((tag("STRING"))) author;
  const char * GTY((tag("STRING"))) isbn;
  struct named_object * GTY((tag("0"))) objects;
  int object_count;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);
typedef void (*destructor_fn)(void *);

/* Struct with callback pointers */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare_callback;
  destructor_fn GTY((skip)) cleanup_callback;
  tree root;
};

/* More complex callback structure */
struct GTY(()) callback_container {
  struct tree_walker GTY((tag("0"))) walker;
  void * GTY((skip)) user_data;
  int traversal_depth;
};

/* ==================== COMPLEX NESTED EXAMPLE ==================== */
/* Structure that combines multiple types */
struct GTY(()) master_container {
  /* TYPE_STRUCT members */
  struct plain_struct GTY((tag("0"))) plain;
  struct outer_struct GTY((tag("0"))) outer;
  
  /* TYPE_UNION */
  union value_union GTY((tag("0"))) value;
  
  /* TYPE_POINTER network */
  struct pointer_network * GTY((tag("0"))) network;
  
  /* TYPE_ARRAY */
  struct var_array_container GTY((tag("0"))) arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node GTY((tag("0"))) lang_node;
  
  /* TYPE_SCALAR */
  my_scalar scalar_count;
  color_enum current_color;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) container_name;
  
  /* TYPE_CALLBACK */
  struct tree_walker GTY((tag("0"))) callbacks;
  
  /* Pointer to undefined TYPE_UNDEFINED */
  struct opaque_type * GTY((tag("0"))) mystery;
};

/* ==================== TYPE_NONE edge case ==================== */
/* This should not appear in normal parsing, but ensure switch has default */

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
