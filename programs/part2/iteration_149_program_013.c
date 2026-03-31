/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_type;  /* This will trigger TYPE_UNDEFINED */

/* TYPE_SCALAR: Basic scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC marking */
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
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct tree_list *GTY((chain_next("%h.next"))) chain_next;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_list **double_ptr;
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Pointer to undefined type */
  void *generic_ptr;
};

/* Self-referential structure */
struct GTY(()) self_ref {
  int data;
  struct self_ref *GTY((skip)) next;
};

/* TYPE_ARRAY: Arrays with different GTY annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_array;
  int str_len;
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
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific structure pattern */
struct GTY((tag("LANG_TYPE"))) lang_type_info {
  tree base_type;
  struct lang_specific_tree_node *extended_info;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((length("strlen(%h.description) + 1"))) description;
  char * GTY((string)) mutable_string;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);
typedef tree (*transform_fn)(tree, void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  compare_fn GTY((skip)) compare;
  void * GTY((skip)) user_data;
};

/* Structure containing multiple callback types */
struct GTY(()) callback_container {
  transform_fn GTY((skip)) transformer;
  void (* GTY((skip)) cleanup)(void *);
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_nested {
  struct plain_struct base;
  union value_union value;
  struct tree_list *list_head;
  struct array_container arrays;
  struct named_object name_info;
  struct tree_walker walker;
  struct lang_specific_tree_node *lang_node;
};

/* Template for parameterized structure (using macros) */
#define DECLARE_GTY_STRUCT(name, field_type) \
  struct GTY(()) name##_container { \
    field_type * GTY((length("count"))) items; \
    int count; \
    const char * GTY((tag("STRING"))) name; \
  }

/* Instantiate the template */
DECLARE_GTY_STRUCT(int, int);
DECLARE_GTY_STRUCT(tree, tree);

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
  int has_extra;
  tree main_value;
  tree * GTY((length("has_extra ? 10 : 0"))) extra_values;
  const char * GTY((tag("STRING"))) GTY((skip)) optional_name;
};

/* Union with pointers */
union GTY(()) ptr_union {
  struct plain_struct *struct_ptr;
  struct tree_list *list_ptr;
  struct array_container *array_ptr;
  void *generic_ptr;
};

/* Structure containing union of pointers */
struct GTY(()) union_ptr_container {
  int type;
  union ptr_union GTY((desc("type"))) ptrs;
};

#endif /* TEST_COVERAGE_H */
