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
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
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

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *prev;              /* Regular pointer */
  struct plain_struct *related;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *children[4];
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("name_len"))) name_chars;
  size_t name_len;
};

/* Multi-dimensional array example */
struct GTY(()) matrix {
  int rows;
  int cols;
  double * GTY((length("rows * cols"))) data;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Mimicking tree structure language-specific nodes */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_flags;
};

struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  int block_number;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("IDENTIFIER"))) identifier;
  const char *description;  /* Regular string pointer */
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT member */
  struct plain_struct base;
  
  /* TYPE_UNION member */
  union value_union data;
  
  /* TYPE_ARRAY member */
  struct array_container arrays;
  
  /* TYPE_POINTER network */
  struct tree_list *GTY((skip)) list_head;
  
  /* TYPE_STRING members */
  const char * GTY((tag("STRING"))) type_name;
  
  /* TYPE_CALLBACK member */
  walk_fn GTY((skip)) visitor;
  
  /* TYPE_SCALAR members */
  my_scalar scalar_field;
  color_enum enum_field;
  
  /* Reference to TYPE_USER_STRUCT */
  struct user_defined *user_data;
  
  /* Reference to TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
};

/* Another structure using the undefined type */
struct GTY(()) uses_opaque {
  struct opaque_type *GTY((skip)) opaque_ptr;  /* TYPE_UNDEFINED reference */
  int known_field;
};

/* Global variables marked with GTY for additional coverage */
extern struct plain_struct * GTY((length("global_count"))) global_array;
extern int global_count;

extern const char * GTY((tag("STRING"))) global_string;

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
