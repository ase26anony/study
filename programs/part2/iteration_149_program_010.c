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
  int id;
  struct plain_struct GTY((tag("inner"))) inner;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* Another user struct with callback */
struct GTY((user)) user_with_callback {
  void (*GTY((skip)) custom_alloc)(void*);
  void *GTY((skip)) user_context;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  double d;
  const char *s;
  tree t;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int type;
  union GTY((desc("type"))) {
    int as_int;
    double as_double;
    tree as_tree;
  } value;
};

/* ==================== TYPE_POINTER ==================== */
/* Self-referential pointer structure */
struct GTY(()) tree_node {
  int code;
  tree GTY((tag("TYPE"))) type;
  struct tree_node *GTY((skip)) next;  /* Skip pointer */
  struct tree_node *GTY((tag("CHILD"))) first_child;
};

/* Complex pointer network */
struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_node **double_ptr;
  struct outer_struct *GTY((skip)) skipped_ptr;
  void *GTY((tag("OPAQUE"))) opaque_ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree static_trees[10];
};

/* Variable-length array with length annotation */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  struct plain_struct ** GTY((length("dynamic_count * 2"))) struct_array;
};

/* Nested array structure */
struct GTY(()) nested_arrays {
  int matrix[3][3];
  tree * GTY((length("row_count"))) rows[10];
  int row_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_flag;
};

/* Another language structure */
struct GTY((tag("TS_DECL_MINIMAL"))) lang_decl_node {
  tree name;
  tree context;
  unsigned lang_specific : 8;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef */
typedef int my_scalar;
typedef unsigned long bitmask_t;

/* Enum type */
enum gty_test_enum {
  GTY_TEST_ZERO,
  GTY_TEST_ONE,
  GTY_TEST_TWO
};

/* Struct with various scalar types */
struct GTY(()) scalar_container {
  my_scalar count;
  bitmask_t flags;
  enum gty_test_enum state;
  long big_number;
  unsigned char small;
  float float_val;
  double double_val;
};

/* ==================== TYPE_STRING ==================== */
/* String field with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* Multiple string types */
struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) static_string;
  char * GTY((tag("STRING"))) mutable_string;
  const char *const * GTY((length("string_count"))) string_array;
  int string_count;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef */
typedef void (*tree_walk_fn)(tree node, void *data);
typedef int (*tree_predicate_fn)(tree node);

/* Struct with callback fields */
struct GTY(()) tree_walker {
  tree_walk_fn GTY((skip)) pre_order;
  tree_walk_fn GTY((skip)) post_order;
  tree_predicate_fn GTY((skip)) filter;
  void * GTY((skip)) user_data;
};

/* Another callback example */
struct GTY(()) callback_container {
  void (*GTY((skip)) alloc_hook)(size_t);
  void (*GTY((skip)) free_hook)(void*);
  int (*GTY((skip)) validate_fn)(const struct plain_struct*);
};

/* ==================== COMPLEX COMBINATIONS ==================== */
/* Structure combining multiple type categories */
struct GTY(()) master_container {
  /* Basic types */
  int id;
  
  /* String */
  const char * GTY((tag("STRING"))) container_name;
  
  /* Struct pointer */
  struct plain_struct *first_struct;
  
  /* Array of pointers */
  struct tree_node ** GTY((length("node_count"))) nodes;
  int node_count;
  
  /* Union */
  union value_union current_value;
  
  /* Language-specific */
  struct lang_specific_tree_node *lang_node;
  
  /* User struct */
  struct user_defined *user_data;
  
  /* Callback */
  tree_walk_fn GTY((skip)) iterator;
  
  /* Scalar enum */
  enum gty_test_enum container_type;
  
  /* Undefined type pointer (should trigger TYPE_UNDEFINED) */
  struct opaque_type *opaque;
  
  /* Nested array of strings */
  const char * GTY((tag("STRING"))) * GTY((length("tag_count"))) tags;
  int tag_count;
};

/* Root structure for GC */
struct GTY(()) gcc_root {
  struct master_container *main;
  struct named_object *symbols;
  struct var_array_container *arrays;
  struct tree_walker *walker;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
