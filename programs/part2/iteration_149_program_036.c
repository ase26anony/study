/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype during GCC build */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED test */
struct GTY(()) opaque_type;  /* This will be TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  enum color color;
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

/* Struct containing a union */
struct GTY(()) union_container {
  int tag;
  union GTY((desc ("tag"))) {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((tag ("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct plain_struct *related;
};

/* Self-referential structure */
struct GTY(()) self_ref {
  int id;
  struct self_ref *GTY((skip)) sibling;
  struct self_ref *GTY((chain_next ("sibling"))) children;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length ("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char ** GTY((length ("string_count"))) strings;
  int string_count;
};

/* Multi-dimensional array example */
struct GTY(()) matrix {
  int rows;
  int cols;
  double * GTY((length ("rows * cols"))) data;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag ("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure pattern */
struct GTY((tag ("TS_BINFO"))) lang_binfo {
  tree base;
  tree inheritance_chain;
};

/* TYPE_STRING: String handling */
struct GTY(()) named_object {
  const char * GTY((tag ("STRING"))) name;
  const char * GTY((length ("strlen(name) + 1"))) name_copy;
  tree decl;
};

/* String with allocation kind */
struct GTY(()) string_pool {
  const char * GTY((string_length ("strlen"))) pooled_string;
  int ref_count;
};

/* TYPE_CALLBACK in struct context */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_type {
  /* Scalar fields */
  my_scalar count;
  my_long_scalar big_count;
  enum color current_color;
  
  /* Struct field */
  struct plain_struct plain;
  
  /* Pointer fields */
  struct tree_list *list_head;
  struct self_ref *root;
  
  /* Array field */
  struct array_container arrays;
  
  /* Union field */
  union value_union current_value;
  
  /* String field */
  struct named_object name_info;
  
  /* Callback field */
  struct tree_walker walker;
  
  /* Language-specific field */
  struct lang_specific_tree_node *lang_node;
  
  /* Pointer to undefined type (for TYPE_UNDEFINED) */
  struct opaque_type *GTY((skip)) opaque_ref;
};

/* Template for generating multiple instances */
struct GTY(()) type_factory {
  struct complex_type * GTY((length ("type_count"))) types;
  int type_count;
  struct user_defined *user_types;  /* This will use user-defined handling */
};

/* Additional pointer variations */
struct GTY(()) pointer_network {
  struct tree_list ** GTY((skip)) list_array;  /* Array of pointers */
  struct complex_type * GTY((chain_next ("next_complex"))) first_complex;
  struct complex_type *next_complex;
};

/* Union with nested structures */
union GTY(()) nested_union {
  struct {
    int header;
    tree payload;
  } GTY((tag ("1"))) structured;
  struct array_container array_data;
  const char * GTY((tag ("STRING"))) raw_string;
};

#endif /* TEST_COVERAGE_H */
