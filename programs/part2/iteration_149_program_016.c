/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

/* Forward declarations to create TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* This will be TYPE_UNDEFINED - never defined */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long;
typedef unsigned int my_uint;

/* Enum type (also scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;        /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* Another union inside a struct */
struct GTY(()) union_container {
  int tag;
  union {
    int int_val;
    tree tree_val;
    const char *str_val;
  } GTY((desc("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *prev;              /* Regular pointer */
};

/* Self-referential structure */
struct GTY(()) linked_node {
  int data;
  struct linked_node *next;
  struct linked_node *prev;
};

/* Pointer to opaque type (TYPE_UNDEFINED) */
struct GTY(()) uses_opaque {
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Pointer to undefined type */
  int valid;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char ** GTY((length("str_count"))) strings;
  int str_count;
};

/* Multi-dimensional array example */
struct GTY(()) matrix {
  int rows;
  int cols;
  double * GTY((length("rows * cols"))) data;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
/* Mimic tree structure with language-specific tags */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  int lang_specific_data;
};

/* Another language-specific structure pattern */
struct GTY((tag("LANG_TYPE"))) lang_type_info {
  tree base_type;
  unsigned int lang_flags;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;  /* Explicit string tag */
  const char *description;                  /* Implicit string */
  tree associated_tree;
};

/* String container with array of strings */
struct GTY(()) string_table {
  const char * GTY((length("table_size"))) * GTY((tag("STRING"))) entries;
  int table_size;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;  /* Skip function pointer */
  compare_fn GTY((skip)) compare;
  void *user_data;
};

/* Combined structure using multiple type kinds */
struct GTY(()) complex_structure {
  /* TYPE_STRUCT elements */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union current_value;
  
  /* TYPE_POINTER network */
  struct tree_list *items;
  struct linked_node *nodes;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_STRING */
  struct named_object name_info;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_SCALAR */
  my_scalar counter;
  enum color color;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_defined *user_data;
};

/* Root structure that ties everything together */
struct GTY(()) coverage_root {
  struct complex_structure main_struct;
  struct matrix *matrices;
  struct string_table *string_tables;
  struct lang_type_info *lang_types;
  struct uses_opaque opaque_user;
};

#endif /* TEST_COVERAGE_H */
