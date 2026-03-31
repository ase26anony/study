/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

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
  tree GTY((skip)) node;  /* Skip this field during GC */
  double value;
};

/* Another struct with nested structures */
struct GTY(()) container_struct {
  struct plain_struct GTY((tag("0"))) item;
  int count;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct that requires special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
  /* User will provide custom marking routines */
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int int_val;
  double double_val;
  const char *string_val;
  tree tree_val;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int type;
  union {
    int i;
    double d;
    tree t;
  } GTY((desc("type"))) value;
};

/* ==================== TYPE_POINTER ==================== */
/* Self-referential pointer structure */
struct GTY(()) tree_node {
  tree value;
  struct tree_node *GTY((skip)) next;  /* Skip pointer */
  struct tree_node *GTY((tag("1"))) left;
  struct tree_node *GTY((tag("2"))) right;
};

/* Complex pointer network */
struct GTY(()) pointer_network {
  struct plain_struct *GTY((tag("0"))) plain_ptr;
  struct tree_node *GTY((tag("1"))) node_ptr;
  void *GTY((skip)) opaque_ptr;  /* Skip this pointer */
  struct pointer_network *GTY((tag("2"))) self_ptr;  /* Self-reference */
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("fixed_count"))) items[10];
  int fixed_count;
};

/* Struct with variable-length array pointer */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  struct plain_struct ** GTY((length("struct_count"))) struct_array;
  int struct_count;
};

/* Nested array structure */
struct GTY(()) nested_array {
  struct var_array_container GTY((tag("0"))) containers[3];
  int container_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("1"))) decl;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure */
struct GTY((tag("TS_COMMON"))) lang_common_node {
  tree chain;
  tree type;
  enum tree_code code : 16;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

/* Enum type */
enum gty_test_enum {
  TEST_ENUM_A,
  TEST_ENUM_B,
  TEST_ENUM_C
};

/* Struct with various scalar types */
struct GTY(()) scalar_container {
  my_scalar count;
  my_long_scalar big_count;
  my_unsigned_scalar flags;
  enum gty_test_enum state;
  char small_scalar;
  short medium_scalar;
  float float_scalar;
  double double_scalar;
};

/* ==================== TYPE_STRING ==================== */
/* Struct with string fields */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree GTY((tag("0"))) decl;
};

/* Another string container */
struct GTY(()) string_pair {
  const char * GTY((tag("STRING"))) key;
  const char * GTY((tag("STRING"))) value;
  struct named_object * GTY((tag("0"))) obj;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);
typedef int (*predicate_fn)(const void*);

/* Struct with callback fields */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void * GTY((skip)) callback_data;
  tree root;
};

/* Another callback container */
struct GTY(()) callback_container {
  predicate_fn GTY((skip)) filter;
  struct tree_walker * GTY((tag("0"))) walker;
  int callback_count;
};

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* This combines multiple types to ensure thorough testing */
struct GTY(()) master_container {
  /* TYPE_STRUCT members */
  struct plain_struct GTY((tag("0"))) plain;
  struct container_struct GTY((tag("1"))) container;
  
  /* TYPE_USER_STRUCT */
  struct user_defined GTY((tag("2"))) user;
  
  /* TYPE_UNION */
  union value_union GTY((tag("3"))) union_val;
  
  /* TYPE_POINTER network */
  struct pointer_network * GTY((tag("4"))) network;
  
  /* TYPE_ARRAY containers */
  struct var_array_container GTY((tag("5"))) arrays[2];
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node GTY((tag("6"))) lang_node;
  
  /* TYPE_SCALAR fields */
  struct scalar_container GTY((tag("7"))) scalars;
  
  /* TYPE_STRING fields */
  struct named_object * GTY((tag("8"))) named_objects[4];
  
  /* TYPE_CALLBACK */
  struct tree_walker GTY((tag("9"))) walker;
  
  /* Direct scalar */
  int total_count;
  
  /* Direct string */
  const char * GTY((tag("STRING"))) master_name;
};

/* ==================== FORWARD DECLARATIONS ==================== */
/* More forward declarations for TYPE_UNDEFINED testing */
struct GTY(()) undefined_struct_1;
struct GTY(()) undefined_struct_2;
union GTY(()) undefined_union;

/* Pointer to undefined type */
struct GTY(()) has_undefined_ptr {
  struct undefined_struct_1 * GTY((skip)) undefined_ptr;
  int defined_field;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
