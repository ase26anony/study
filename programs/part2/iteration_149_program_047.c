/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

/* Forward declarations to create TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - TYPE_UNDEFINED */

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
  tree GTY((skip)) node;  /* Skip this field during GC */
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

/* TYPE_ARRAY: Structs with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Nested array in a pointer */
  struct plain_struct ** GTY((length("nested_count"))) nested_array;
  
  int dynamic_count;
  int nested_count;
};

/* Another array example with 2D array */
struct GTY(()) matrix {
  int rows;
  int cols;
  double * GTY((length("rows * cols"))) data;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip pointer */
  struct tree_list * GTY((chain_next("next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) node {
  int id;
  struct node * GTY((skip)) parent;
  struct node * GTY((length("child_count"))) children;
  int child_count;
};

/* Pointer to opaque type (creates TYPE_UNDEFINED reference) */
struct GTY(()) uses_opaque {
  struct opaque_type * GTY((tag("OPAQUE"))) opaque_ptr;
  int valid;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimic tree structure used by GCC frontends */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  source_location locus;
};

/* Another language-specific structure */
struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  source_location start_location;
  source_location end_location;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) filename;
  int line_number;
};

/* String with length attribute */
struct GTY(()) string_with_len {
  const char * GTY((tag("STRING"), length("strlen(name) + 1"))) name;
  size_t strlen;
};

/* TYPE_CALLBACK in a struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void * GTY((skip)) user_data;
};

/* Combined structure using multiple types */
struct GTY(()) complex_type {
  /* Scalar fields */
  my_scalar count;
  my_long_scalar big_count;
  enum color current_color;
  
  /* Struct field */
  struct plain_struct GTY((tag("plain"))) plain;
  
  /* Union field */
  union value_union GTY((tag("value"))) val;
  
  /* Array field */
  struct array_container GTY((tag("arrays"))) arrays;
  
  /* Pointer fields */
  struct node * GTY((skip)) root;
  struct tree_list * GTY((chain_next("next"))) list_head;
  
  /* String field */
  const char * GTY((tag("STRING"))) description;
  
  /* Callback field */
  walk_fn GTY((skip)) notify_callback;
  
  /* Reference to user-defined type */
  struct user_defined * GTY((tag("USER"))) user_data;
  
  /* Language-specific structure */
  struct lang_specific_tree_node GTY((tag("LANG"))) lang_node;
};

/* Union containing various types */
union GTY(()) any_value {
  struct plain_struct GTY((tag("STRUCT"))) as_struct;
  union value_union GTY((tag("UNION"))) as_union;
  struct array_container GTY((tag("ARRAY"))) as_array;
  struct node * GTY((tag("POINTER"))) as_pointer;
  const char * GTY((tag("STRING"))) as_string;
  walk_fn GTY((tag("CALLBACK"))) as_callback;
};

/* Template-like structure for testing parameterized types */
#define DECLARE_CONTAINER(TYPE, NAME) \
  struct GTY(()) NAME { \
    TYPE * GTY((length("count"))) items; \
    int count; \
    int capacity; \
  }

DECLARE_CONTAINER(tree, tree_vector);
DECLARE_CONTAINER(struct plain_struct, struct_vector);
DECLARE_CONTAINER(union value_union, union_vector);

/* Nested structures for depth testing */
struct GTY(()) outer_struct {
  int outer_id;
  struct GTY(()) middle_struct {
    int middle_id;
    struct GTY(()) inner_struct {
      int inner_id;
      tree value;
    } inner;
    struct inner_struct * GTY((skip)) inner_ptr;
  } middle;
  struct middle_struct * GTY((length("middle_count"))) middle_array;
  int middle_count;
};

#endif /* TEST_COVERAGE_H */
