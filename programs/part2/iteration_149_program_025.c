/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in examples */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Basic scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;  /* User handles memory management */
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  tree t;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer in GC chain */
  struct plain_struct *related;
};

/* Self-referential structure */
struct GTY(()) linked_node {
  int id;
  struct linked_node *GTY((chain_next("%h.next"))) next;
  struct linked_node *prev;  /* Will be marked as skip automatically */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length annotation */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Another variable-length array pointing to structs */
  struct plain_struct ** GTY((length("struct_count"))) struct_array;
  
  /* Length fields */
  int dynamic_count;
  unsigned int struct_count;
  
  /* Nested array in a struct */
  struct {
    short GTY((length("nested_len"))) nested_array[10];
    int nested_len;
  } nested;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking tree structure language hooks */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure pattern */
struct GTY((tag("LANG_TYPE"))) c_type {
  tree main_variant;
  tree variants;
  struct c_type * GTY((skip)) next_variant;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree decl;
};

/* TYPE_CALLBACK usage in struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;  /* Function pointers are skipped */
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
  tree root;
};

/* Complex nested structure to test multiple type combinations */
struct GTY(()) complex_container {
  /* Various pointer types */
  struct plain_struct *plain_ptr;
  struct tree_list *list_head;
  struct linked_node *node_chain;
  
  /* Arrays */
  struct array_container arrays[2];
  
  /* Union */
  union value_union current_value;
  
  /* String */
  const char * GTY((tag("STRING"))) identifier;
  
  /* Scalar types */
  my_scalar scalar_field;
  my_long_scalar long_field;
  color_enum enum_field;
  
  /* Callback */
  walk_fn GTY((skip)) traverse_fn;
  
  /* Nested structure */
  struct {
    int nested_id;
    tree nested_tree;
  } GTY((tag("nested"))) inner;
};

/* Container that references the undefined type */
struct GTY(()) uses_opaque {
  struct opaque_type * GTY((skip)) opaque_ref;  /* TYPE_UNDEFINED reference */
  int known_field;
};

/* Union containing various pointer types */
union GTY(()) pointer_union {
  struct plain_struct *plain;
  struct tree_list *list;
  struct linked_node *node;
  tree tree_ptr;
  const char * GTY((tag("STRING"))) string_ptr;
};

/* Array of unions */
struct GTY(()) union_array_container {
  union pointer_union GTY((length("union_count"))) unions[8];
  int union_count;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
  tree base;
  
  /* Conditional pointer based on tree code */
  union {
    struct plain_struct * GTY((tag("if (TREE_CODE(%h.base) == INTEGER_CST)"))) int_info;
    struct named_object * GTY((tag("if (TREE_CODE(%h.base) == IDENTIFIER_NODE)"))) name_info;
    struct array_container * GTY((tag("default"))) default_info;
  } GTY((desc("TREE_CODE(%0.base)"))) variant;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
