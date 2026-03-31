/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file defines various GTY-marked types to exercise all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in examples */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - triggers undefined type handling */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC marking */
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char * GTY((tag("STRING"))) s;
  double d;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip pointer */
  struct tree_list * GTY((chain_next ("%h.next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node * GTY((skip)) parent;
  struct recursive_node * GTY((chain_next ("%h.next"))) children;
  struct recursive_node *next;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length ("%h.dynamic_count"))) var_array;
  int dynamic_count;
  
  /* Nested array in a struct */
  struct GTY(()) nested {
    char * GTY((length ("%h.len"))) data;
    int len;
  } nested_array;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
/* Mimic tree structure with language-specific tags */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure pattern */
struct GTY((tag("LANG_TYPE"))) lang_type_info {
  tree base_type;
  struct lang_type_info *next;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree associated_tree;
};

/* Container with multiple string types */
struct GTY(()) string_container {
  /* Direct string pointer */
  const char * GTY((tag("STRING"))) direct_str;
  
  /* Array of strings */
  const char * GTY((length ("%h.str_count"), tag("STRING"))) *string_array;
  int str_count;
};

/* TYPE_CALLBACK in a struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void * GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) complex_type {
  /* Scalar fields */
  my_scalar id;
  color_enum color;
  
  /* Struct field */
  struct plain_struct embedded;
  
  /* Union field */
  union value_union value;
  
  /* Pointer to array */
  struct array_container * GTY((skip)) array_ptr;
  
  /* String field */
  const char * GTY((tag("STRING"))) type_name;
  
  /* Callback field */
  walk_fn GTY((skip)) validator;
  
  /* Self-reference */
  struct complex_type *next;
};

/* Additional pointer types for coverage */
struct GTY(()) pointer_network {
  /* Multiple pointer types in one struct */
  tree * GTY((length ("%h.tree_count"))) tree_array;
  int tree_count;
  
  struct plain_struct *struct_ptr;
  struct user_defined * GTY((skip)) user_ptr;
  struct lang_specific_tree_node *lang_ptr;
  
  /* Pointer to callback structure */
  struct tree_walker * GTY((skip)) walker;
  
  /* Pointer to string container */
  struct string_container *strings;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct plain_struct *struct_ptr;
  struct array_container *array_ptr;
  const char * GTY((tag("STRING"))) string_ptr;
};

#endif /* TEST_COVERAGE_H */
