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
  int user_tag;
};

/* Another user struct example */
struct GTY((user)) custom_allocated {
  void* GTY((skip)) raw_buffer;
  size_t buffer_size;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  double d;
  const char* GTY((tag("STRING"))) s;
  tree t;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_val;
    double double_val;
    tree GTY((tag("1"))) tree_val;
  } GTY((desc("type"))) u;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer network with self-referential pointers */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer in GC */
  struct tree_list *GTY((tag("0"))) prev;
};

/* Pointer to another GTY-marked struct */
struct GTY(()) pointer_network {
  struct plain_struct *GTY((tag("0"))) plain_ptr;
  struct tree_list *GTY((tag("0"))) list_ptr;
  void *GTY((skip)) opaque_ptr;  /* Skip this pointer */
};

/* Multiple levels of indirection */
struct GTY(()) deep_pointers {
  struct pointer_network **GTY((tag("2"))) network_pptr;
  tree *GTY((tag("1"))) tree_array_ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("fixed_count"))) items[10];
  int fixed_count;
};

/* Struct with variable-length array using pointer */
struct GTY(()) var_array_container {
  int dynamic_count;
  tree * GTY((length("dynamic_count"))) var_array;
  const char ** GTY((length("str_count"))) strings;
  int str_count;
};

/* Nested arrays */
struct GTY(()) nested_arrays {
  struct fixed_array_container GTY((length("container_count"))) *containers;
  int container_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned lang_flag1 : 1;
  unsigned lang_flag2 : 1;
};

/* Another language-specific struct */
struct GTY((tag("TS_BINFO"))) base_binfo {
  tree base;
  tree offsets;
  tree virtuals;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedef */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int bitmask_t;

/* Enum type */
enum gty_test_enum {
  GTY_TEST_ZERO,
  GTY_TEST_ONE,
  GTY_TEST_TWO
};

/* Struct with scalar fields */
struct GTY(()) has_scalars {
  my_scalar count;
  my_long_scalar big_count;
  bitmask_t flags;
  enum gty_test_enum state;
  char small_int : 4;
  unsigned char tiny_int : 2;
};

/* ==================== TYPE_STRING ==================== */
/* String field with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* Multiple string fields */
struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) filename;
  const char * GTY((tag("STRING"))) * GTY((length("string_count"))) strings;
  int string_count;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

/* Struct with callback field */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare_callback;
  void * GTY((skip)) user_data;
};

/* More complex callback structure */
struct GTY(()) callback_container {
  struct tree_walker *GTY((tag("0"))) walker;
  void (* GTY((skip)) cleanup_fn)(void*);
};

/* ==================== COMPLEX COMBINATIONS ==================== */
/* Struct combining multiple type categories */
struct GTY(()) master_container {
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER */
  struct tree_list *GTY((tag("0"))) items;
  
  /* TYPE_ARRAY */
  tree * GTY((length("tree_count"))) tree_array;
  int tree_count;
  
  /* TYPE_SCALAR */
  my_scalar magic_number;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) container_name;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) iterator;
  
  /* Reference to undefined type */
  struct opaque_type *GTY((skip)) opaque_ref;
};

/* ==================== TYPE_LANG_STRUCT variations ==================== */
/* Additional language structs to ensure coverage */
struct GTY((tag("TS_VEC"))) tree_vec {
  int length;
  tree GTY((length("length"))) a[1];
};

struct GTY((tag("TS_COMMON"))) tree_common {
  tree chain;
  tree type;
  enum tree_code code : 8;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
