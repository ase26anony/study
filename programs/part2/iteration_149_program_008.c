/* test-coverage.h - Header file to test gengtype state generation coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* This will be TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip tree node for GC */
  my_scalar count;
  enum color col;
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
  struct plain_struct *nested;  /* Pointer to another GTY struct */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  int list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
  struct GTY((tag("TS_COMMON"))) lang_common {
    tree chain;
    tree type;
  } common;
};

/* TYPE_STRING: String type with tag attribute */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("ID"))) identifier;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef void (*traverse_fn)(tree, void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  traverse_fn GTY((skip)) traverse;
  void * GTY((skip)) data;
};

/* Additional complex structures to ensure coverage */

/* Nested structures with various types */
struct GTY(()) complex_container {
  struct plain_struct plain;
  union value_union val;
  struct named_object obj;
  struct tree_walker walker;
  struct array_container arrays;
};

/* Pointer chain structure */
struct GTY(()) pointer_chain {
  struct pointer_chain *GTY((skip)) next;
  struct pointer_chain *prev;
  void *data;
};

/* Union within struct */
struct GTY(()) union_container {
  int type;
  union {
    int int_val;
    const char *str_val;
    tree tree_val;
  } GTY((desc("type"))) value;
};

/* Array of pointers */
struct GTY(()) pointer_array {
  tree * GTY((length("count"))) nodes;
  struct plain_struct ** GTY((length("struct_count"))) structs;
  int count;
  int struct_count;
};

/* For TYPE_UNDEFINED - forward declaration without definition */
struct GTY(()) another_opaque;

#endif /* TEST_COVERAGE_H */
