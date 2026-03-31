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
/* Forward declaration of opaque type - never defined */
struct GTY(()) opaque_type;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;

enum color { RED, GREEN, BLUE };

/* ==================== TYPE_STRUCT ==================== */
/* Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  enum color color;
};

/* Another struct for pointer relationships */
struct GTY(()) another_struct {
  int id;
  const char* GTY((tag("STRING"))) description;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct requiring special handling */
struct GTY((user)) user_defined {
  void* private_data;  /* User handles memory management */
  int user_tag;
};

/* ==================== TYPE_UNION ==================== */
/* Standalone GTY-marked union */
union GTY(()) value_union {
  int i;
  double d;
  const char* GTY((tag("STRING"))) s;
};

/* Union inside a struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_val;
    tree GTY((tag("TREE"))) tree_val;
    const char* GTY((tag("STRING"))) str_val;
  } GTY((desc("type"))) value;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer network with self-referential pointers */
struct GTY(()) tree_list {
  tree value;
  struct tree_list* GTY((skip)) next;  /* Skip pointer for manual management */
  struct tree_list* GTY((chain_next("%h.next"))) chain_next;
};

/* Pointer to another GTY-marked struct */
struct GTY(()) struct_network {
  struct plain_struct* GTY((skip)) plain_ptr;
  struct another_struct* another_ptr;
  struct tree_list* list_head;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree* GTY((length("dynamic_count"))) var_array;
  
  /* Array of pointers */
  struct plain_struct** GTY((length("ptr_count"))) ptr_array;
  
  /* Nested array */
  int GTY((length("rows * cols"))) matrix;
  
  /* Count fields for variable arrays */
  int dynamic_count;
  int ptr_count;
  int rows;
  int cols;
};

/* Array of unions */
struct GTY(()) union_array {
  union value_union GTY((length("union_count"))) unions[10];
  int union_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_flag;
};

/* Another language struct with different tag */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree* GTY((length("vtable_size"))) vtable;
  int vtable_size;
};

/* ==================== TYPE_STRING ==================== */
/* String type handling */
struct GTY(()) named_object {
  const char* GTY((tag("STRING"))) name;
  const char* GTY((tag("STRING"))) filename;
  int line_number;
};

/* String in union context */
struct GTY(()) string_container {
  int string_type;
  union {
    const char* GTY((tag("STRING"))) c_string;
    char* GTY((tag("STRING"))) mutable_string;
  } GTY((desc("string_type"))) str_data;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

/* Struct with callback pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare_callback;
  void* GTY((skip)) user_data;
};

/* More complex callback structure */
struct GTY(()) traversal_state {
  struct tree_walker* GTY((skip)) walker;
  tree current_node;
  int depth;
};

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* Structure that combines multiple types */
struct GTY(()) master_container {
  /* Scalar types */
  my_scalar count;
  my_long_scalar big_count;
  enum color default_color;
  
  /* Structures */
  struct plain_struct plain;
  struct another_struct another;
  
  /* User struct */
  struct user_defined* user_data;
  
  /* Unions */
  union value_union current_value;
  struct union_container union_wrapper;
  
  /* Pointers and networks */
  struct tree_list* item_list;
  struct struct_network* network;
  
  /* Arrays */
  struct array_container arrays;
  struct union_array union_arrays;
  
  /* Language-specific */
  struct lang_specific_tree_node* lang_node;
  struct lang_binfo* binfo;
  
  /* Strings */
  struct named_object name_info;
  struct string_container string_wrapper;
  
  /* Callbacks */
  struct tree_walker walker;
  struct traversal_state* traversal;
  
  /* Undefined type pointer */
  struct opaque_type* GTY((skip)) opaque_ptr;
};

/* Global variable declarations for gengtype to process */
extern struct master_container GTY(()) global_container;
extern struct tree_list* GTY((chain_next("%h.next"))) global_list;

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
