/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED */
struct GTY(()) opaque_type;  /* Never defined - triggers TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;        /* TYPE_SCALAR usage */
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: C union within GTY context */
union GTY(()) value_union {
  int i;
  const char *s;
  tree t;
};

struct GTY(()) union_container {
  int tag;
  union value_union GTY((desc("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Self-referential pointer */
  struct plain_struct *related;        /* Pointer to another GTY struct */
};

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
  const char ** GTY((length("str_count"))) strings;
  int str_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  struct lang_block *chain;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;  /* Explicit string tag */
  const char *description;                  /* Implicit string */
};

/* TYPE_CALLBACK: Function pointer in GTY struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void *GTY((skip)) user_data;
};

/* Composite structure using multiple type kinds */
struct GTY(()) composite_type {
  /* TYPE_STRUCT elements */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union current_value;
  
  /* TYPE_POINTER */
  struct tree_list *items;
  struct opaque_type *GTY((skip)) opaque_ref;  /* TYPE_UNDEFINED reference */
  
  /* TYPE_ARRAY */
  struct array_container containers[2];
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node lang_node;
  
  /* TYPE_STRING */
  struct named_object naming;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_SCALAR */
  my_scalar id;
  my_long_scalar big_id;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user_data;
};

/* Another union type definition */
union GTY(()) another_union {
  struct plain_struct *ps;
  struct array_container *ac;
  tree single_tree;
};

#endif /* TEST_COVERAGE_H */
