/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Self-referential pointer with skip */
  struct plain_struct *nested;          /* Pointer to another GTY struct */
};

/* More complex pointer structure */
struct GTY(()) pointer_network {
  struct tree_list *head;
  struct tree_list **GTY((skip)) tail_ptr;  /* Pointer to pointer */
  void *GTY((skip)) opaque_ptr;             /* Opaque pointer */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_array;  /* String array */
  int str_len;
};

/* Multi-dimensional array example */
struct GTY(()) matrix_container {
  int * GTY((length("rows * cols"))) matrix;
  int rows;
  int cols;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  enum color color_tag;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_COMMON"))) common_tree_node {
  tree chain;
  tree type;
  enum tree_code code : 8;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef void (*traverse_fn)(tree, void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  traverse_fn GTY((skip)) traverse;
  void * GTY((skip)) data;
};

/* Composite structure using all types */
struct GTY(()) composite_type {
  /* Scalar fields */
  my_scalar id;
  my_long_scalar timestamp;
  enum color color;
  
  /* Struct fields */
  struct plain_struct plain;
  struct user_defined *user;  /* Pointer to user struct */
  
  /* Union field */
  union value_union value;
  
  /* Pointer fields */
  struct tree_list *list;
  struct pointer_network *network;
  
  /* Array fields */
  struct array_container arrays;
  
  /* Language-specific field */
  struct lang_specific_tree_node *lang_node;
  
  /* String field */
  struct named_object naming;
  
  /* Callback field */
  struct tree_walker walker;
  
  /* Undefined type reference (forward declaration) */
  struct opaque_type *opaque_ref;
};

/* Additional test structures */

/* Nested structures */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int x;
    int y;
  } inner;
  struct inner_struct *inner_ptr;
};

/* Union within struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_value;
    double double_value;
    const char *string_value;
  } GTY((desc("type"))) data;
};

/* Array of pointers */
struct GTY(()) pointer_array {
  tree * GTY((length("count"))) nodes;
  struct plain_struct ** GTY((length("struct_count"))) structs;
  int count;
  int struct_count;
};

/* Chain of structures for traversal testing */
struct GTY(()) chain_link {
  int id;
  struct chain_link *GTY((skip)) next;
  struct chain_link *prev;
};

#endif /* TEST_COVERAGE_H */
