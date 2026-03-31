/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

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

/* Another struct with nested structure */
struct GTY(()) outer_struct {
  struct plain_struct GTY((tag("0"))) inner;
  unsigned count;
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
  int type_tag;
  union {
    int int_val;
    double double_val;
    tree GTY((tag("1"))) tree_val;
  } GTY((desc("type_tag"))) u;
};

/* ==================== TYPE_POINTER ==================== */
/* Self-referential pointer structure */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip pointer in GC */
  struct tree_list * GTY((tag("0"))) prev;
};

/* Complex pointer network */
struct GTY(()) pointer_network {
  struct plain_struct * GTY((tag("0"))) plain_ptr;
  struct outer_struct ** GTY((tag("1"))) outer_pptr;
  struct tree_list * GTY((chain_next("next"), chain_prev("prev"))) list_head;
  void * GTY((skip)) opaque_ptr;  /* Skip this pointer */
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("fixed[0]"))) fixed_trees[10];  /* Fixed length reference */
};

/* Struct with variable-length array */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  struct plain_struct * GTY((length("dynamic_count * 2"))) struct_array;
};

/* Nested array structure */
struct GTY(()) nested_arrays {
  int matrix[3][4];
  tree * GTY((length("row_count"))) * GTY((length("col_count"))) jagged;
  int row_count;
  int col_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned lang_flag : 1;
  unsigned lang_data : 31;
};

/* Another language structure with different tag */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree vtable;
  unsigned inheritance_depth;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;

/* Enum type */
enum gty_test_enum {
  GTY_TEST_ZERO,
  GTY_TEST_ONE,
  GTY_TEST_TWO
};

/* Struct with various scalar types */
struct GTY(()) has_scalars {
  my_scalar count;
  my_unsigned_scalar size;
  enum gty_test_enum state;
  char small;
  short medium;
  long large;
  float f;
  double d;
  _Bool flag;
};

/* ==================== TYPE_STRING ==================== */
/* String field with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* Multiple string types */
struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) literal;
  char * GTY((tag("STRING"))) mutable_str;
  unsigned char * GTY((tag("STRING"))) binary_data;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

/* Struct with callback fields */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order;
  walk_fn GTY((skip)) post_order;
  compare_fn GTY((skip)) comparator;
  void * GTY((skip)) user_data;
};

/* Callback in union */
struct GTY(()) callback_container {
  int type;
  union {
    walk_fn GTY((skip)) walker;
    compare_fn GTY((skip)) comparer;
    void (* GTY((skip)) generic)(void);
  } callback;
};

/* ==================== COMPLEX COMBINATIONS ==================== */
/* Structure combining multiple type categories */
struct GTY(()) master_structure {
  /* TYPE_STRUCT nested */
  struct plain_struct GTY((tag("0"))) base;
  
  /* TYPE_UNION */
  union value_union GTY((tag("1"))) data;
  
  /* TYPE_POINTER network */
  struct pointer_network * GTY((tag("2"))) ptr_net;
  
  /* TYPE_ARRAY */
  struct var_array_container GTY((tag("3"))) arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node GTY((tag("4"))) lang_node;
  
  /* TYPE_SCALAR */
  my_scalar counter;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) identifier;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) iterator;
  
  /* Reference to TYPE_USER_STRUCT */
  struct user_defined * GTY((tag("5"))) user_data;
  
  /* Pointer to TYPE_UNDEFINED */
  struct opaque_type * GTY((skip)) opaque_ref;
};

/* Root structure for GC */
struct GTY(()) root_container {
  struct master_structure * GTY((tag("0"))) master;
  struct tree_list * GTY((chain_next("next"))) all_lists;
  struct named_object * GTY((tag("1"))) named_objects[10];
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
