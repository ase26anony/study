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
/* This will trigger write_state_undefined_type() */

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
struct GTY(()) container_with_union {
  int type;
  union {
    int int_val;
    double double_val;
    tree GTY((tag("1"))) tree_val;
  } GTY((tag("2"))) data;
};

/* ==================== TYPE_POINTER ==================== */
/* Complex pointer network with self-referential pointers */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for manual linking */
  struct tree_list *GTY((tag("3"))) prev;
};

/* Pointer to another GTY-marked struct */
struct GTY(()) graph_node {
  int id;
  struct graph_node** GTY((length("neighbor_count"))) neighbors;
  int neighbor_count;
  struct plain_struct* GTY((tag("4"))) associated_data;
};

/* Multiple pointer types in one struct */
struct GTY(()) pointer_network {
  void* GTY((skip)) opaque_ptr;
  const char* GTY((tag("STRING"))) name;
  struct tree_list* GTY((tag("5"))) list_head;
  struct graph_node** GTY((length("node_count"))) nodes;
  int node_count;
};

/* ==================== TYPE_ARRAY ==================== */
/* Struct with fixed-size array */
struct GTY(()) fixed_array_container {
  int fixed[5];
  tree GTY((length("tree_count"))) trees[10];
  int tree_count;
};

/* Struct with variable-length array using pointer */
struct GTY(()) dynamic_array_container {
  int* GTY((length("int_count"))) int_array;
  tree* GTY((length("dynamic_count"))) var_array;
  const char** GTY((length("name_count"))) names;
  int int_count;
  int dynamic_count;
  int name_count;
};

/* Nested arrays */
struct GTY(()) nested_arrays {
  struct plain_struct GTY((length("struct_count"))) structs[8];
  union value_union* GTY((length("union_count"))) unions;
  int struct_count;
  int union_count;
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure mimicking GCC frontend patterns */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure */
struct GTY((tag("TS_DECL_MINIMAL"))) lang_decl_node {
  tree decl;
  const char* GTY((tag("STRING"))) filename;
  unsigned int line;
};

/* ==================== TYPE_SCALAR ==================== */
/* Scalar typedefs */
typedef int my_scalar;
typedef unsigned long ulong_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* Struct with various scalar types */
struct GTY(()) scalar_container {
  my_scalar count;
  ulong_scalar big_count;
  color_enum color;
  float float_val;
  double double_val;
  char small_int;
  unsigned char byte;
  short short_val;
  unsigned short word;
};

/* ==================== TYPE_STRING ==================== */
/* Struct with string fields */
struct GTY(()) named_object {
  const char* GTY((tag("STRING"))) name;
  const char* GTY((tag("STRING"))) description;
};

/* Multiple string types */
struct GTY(()) string_collection {
  const char* GTY((tag("STRING"))) primary;
  const char* GTY((tag("STRING"))) secondary[3];
  const char** GTY((length("extra_count"), tag("STRING"))) extra_strings;
  int extra_count;
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedefs */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void*, const void*);
typedef void* (*alloc_fn)(size_t);

/* Struct with callback fields */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare;
};

/* More complex callback structure */
struct GTY(()) callback_manager {
  alloc_fn GTY((skip)) allocator;
  void (*GTY((skip)) destructor)(void*);
  struct tree_walker* GTY((tag("6"))) walker;
};

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* Combining multiple types in one complex structure */
struct GTY(()) master_container {
  /* TYPE_STRUCT members */
  struct plain_struct GTY((tag("7"))) base;
  
  /* TYPE_UNION */
  union value_union GTY((tag("8"))) current_value;
  
  /* TYPE_POINTER network */
  struct tree_list* GTY((tag("9"))) item_list;
  struct graph_node* GTY((tag("10"))) graph;
  
  /* TYPE_ARRAY */
  struct dynamic_array_container GTY((tag("11"))) arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node GTY((tag("12"))) lang_node;
  
  /* TYPE_SCALAR */
  my_scalar item_count;
  color_enum status;
  
  /* TYPE_STRING */
  const char* GTY((tag("STRING"))) container_name;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) iteration_callback;
  
  /* Reference to TYPE_USER_STRUCT */
  struct user_defined* GTY((tag("13"))) user_data;
  
  /* Pointer to TYPE_UNDEFINED (opaque) */
  struct opaque_type* GTY((skip)) opaque_handle;
};

/* ==================== TYPE NONE (should not appear) ==================== */
/* TYPE_NONE is for internal use only and should not be triggered by
   user-defined types. It represents an uninitialized type state. */

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
