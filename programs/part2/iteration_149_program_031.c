/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== TYPE_UNDEFINED ==================== */
/* Forward declaration of opaque type - never defined */
struct GTY(()) opaque_type;

/* ==================== TYPE_STRUCT ==================== */
/* Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during marking */
  struct plain_struct *GTY((skip)) next;  /* Skip pointer */
};

/* Another struct with nested structures */
struct GTY(()) outer_struct {
  struct plain_struct GTY((tag("inner"))) inner;
  int count;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void *private_data;  /* User handles memory management */
  int user_tag;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* Union within a struct */
struct GTY(()) container_with_union {
  int type;
  union {
    int int_val;
    tree tree_val;
    const char *str_val;
  } GTY((desc("type"))) value;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer network with self-referential pointers */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *prev;              /* Regular GTY pointer */
};

/* Pointer chain with multiple levels */
struct GTY(()) pointer_network {
  struct tree_list *GTY((tag("list"))) head;
  struct outer_struct *outer;
  void *GTY((skip)) raw_ptr;  /* Skip raw void pointer */
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Another variable-length array referencing another field */
  struct plain_struct ** GTY((length("struct_count"))) struct_array;
  
  int dynamic_count;
  int struct_count;
  
  /* Array of pointers with skip */
  void ** GTY((length("skip_count"), skip)) skip_array;
  int skip_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree GTY((length("vtable_count"))) *vtable;
  int vtable_count;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedefs */
typedef int my_scalar;
typedef unsigned long my_ulong;
typedef enum { RED, GREEN, BLUE } color_enum;

/* Struct with scalar types */
struct GTY(()) has_scalars {
  my_scalar count;
  my_ulong size;
  color_enum color;
  float float_val;
  double double_val;
  _Bool flag;
};

/* ==================== TYPE_STRING ==================== */
/* String types with various attributes */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"), length("desc_len"))) description;
  int desc_len;
};

/* Another string container */
struct GTY(()) string_pair {
  const char * GTY((tag("STRING"))) first;
  const char * GTY((tag("STRING"))) second;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);
typedef void (*cleanup_fn)(void *);

/* Struct with callback pointers */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
};

/* Another callback container */
struct GTY(()) callback_manager {
  cleanup_fn GTY((skip)) cleanup;
  struct tree_walker *walker;
};

/* ==================== COMPLEX COMBINATIONS ==================== */
/* Struct combining multiple type categories */
struct GTY(()) complex_type {
  /* TYPE_STRUCT nested */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER network */
  struct pointer_network *network;
  
  /* TYPE_ARRAY */
  struct array_container GTY((tag("arrays"))) containers[2];
  
  /* TYPE_SCALAR */
  my_scalar id;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) label;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) processor;
  
  /* Reference to TYPE_USER_STRUCT */
  struct user_defined *user_data;
  
  /* Reference to TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
};

/* Chain structure for testing traversal */
struct GTY(()) type_chain {
  struct complex_type *current;
  struct type_chain *next;
  struct type_chain *prev;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
