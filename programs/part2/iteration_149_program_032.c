/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

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

/* Another struct with nested structures */
struct GTY(()) outer_struct {
  struct plain_struct GTY((tag("0"))) inner;
  unsigned long id;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_flags;
};

/* Another user struct with callback */
struct GTY((user)) user_with_callback {
  void (*GTY((skip)) user_func)(void *);
  struct user_defined* GTY((tag("1"))) udata;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
  void *p;
};

/* Union within a struct */
struct GTY(()) union_container {
  int tag;
  union GTY((desc("tag"))) {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((tag("tag"))) value;
};

/* ==================== TYPE_POINTER ==================== */
/* Self-referential pointer structure */
struct GTY(()) tree_node {
  tree value;
  struct tree_node *GTY((skip)) next;  /* Skip pointer */
  struct tree_node *GTY((tag("2"))) left;
  struct tree_node *GTY((tag("3"))) right;
};

/* Complex pointer network */
struct GTY(()) pointer_network {
  struct plain_struct *GTY((tag("4"))) plain_ptr;
  struct outer_struct **GTY((tag("5"))) outer_pptr;
  struct tree_node *GTY((chain_next ("next"))) chain_head;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("fixed_count"))) items[10];
  int fixed_count;
};

/* Struct with variable-length array */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  struct plain_struct ** GTY((length("struct_count"))) struct_array;
  int struct_count;
};

/* Nested array structure */
struct GTY(()) nested_arrays {
  int matrix[3][3];
  char * GTY((length("str_count"))) strings[10];
  int str_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned int lang_specific_flags;
};

/* Another language structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base_types;
  tree vtable;
  unsigned int inheritance_depth;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_ulong;
typedef enum { RED, GREEN, BLUE } color_enum;

/* Struct with various scalar types */
struct GTY(()) scalar_container {
  my_scalar count;
  my_ulong size;
  color_enum color;
  float precision;
  double accuracy;
  char small;
  short medium;
  long large;
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
  const char * GTY((tag("STRING"))) * GTY((length("num_strings"))) strings;
  int num_strings;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);
typedef void (*cleanup_fn)(void *);

/* Struct with callback pointers */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order;
  walk_fn GTY((skip)) post_order;
  compare_fn GTY((skip)) comparator;
  void * GTY((skip)) user_data;
};

/* Another callback container */
struct GTY(()) callback_manager {
  cleanup_fn GTY((skip)) cleanup;
  void (*GTY((skip)) notify)(int, const char *);
  struct tree_walker *GTY((tag("6"))) walker;
};

/* ==================== COMPLEX COMBINATIONS ==================== */
/* Structure combining multiple type categories */
struct GTY(()) master_container {
  /* TYPE_STRUCT members */
  struct plain_struct plain;
  struct outer_struct outer;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *GTY((tag("7"))) user;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER network */
  struct pointer_network *GTY((tag("8"))) network;
  
  /* TYPE_ARRAY containers */
  struct fixed_array_container fixed_arrays;
  struct var_array_container *GTY((tag("9"))) var_arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node lang_node;
  
  /* TYPE_SCALAR fields */
  my_scalar scalar_field;
  color_enum enum_field;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) container_name;
  
  /* TYPE_CALLBACK */
  struct tree_walker *GTY((tag("10"))) walker;
  
  /* Forward reference to undefined type */
  struct opaque_type *GTY((tag("11"))) opaque_ref;
};

/* Global variables for testing */
extern struct master_container GTY((tag("12"))) global_container;
extern struct tree_node * GTY((tag("13"))) global_tree_list;
extern union value_union GTY((tag("14"))) global_union;

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
