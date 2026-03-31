/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* This will remain undefined */

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
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

/* TYPE_ARRAY: Structs with arrays of different types */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len"))) strings;
  size_t str_len;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Self-referential pointer with skip */
  struct plain_struct *nested;  /* Pointer to another GTY struct */
};

struct GTY(()) pointer_network {
  struct tree_list *head;
  struct array_container *container;
  struct pointer_network *GTY((chain_next("%h.next"))) next;
  void * GTY((skip)) raw_ptr;  /* Skip raw pointer */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  struct lang_binfo *next;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* More complex structure combining multiple types */
struct GTY(()) composite_type {
  /* TYPE_STRUCT elements */
  struct plain_struct plain;
  
  /* TYPE_UNION */
  union value_union value;
  
  /* TYPE_ARRAY */
  struct array_container * GTY((length("container_count"))) containers;
  int container_count;
  
  /* TYPE_POINTER */
  struct tree_list *items;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  color_enum color;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) identifier;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) walker;
  compare_fn GTY((skip)) comparator;
  
  /* Pointer to undefined type */
  struct opaque_type *opaque_ptr;  /* TYPE_UNDEFINED reference */
};

/* Another union type for additional coverage */
union GTY(()) another_union {
  struct plain_struct *ps;
  struct array_container *ac;
  struct named_object *no;
};

/* Structure with nested anonymous union */
struct GTY(()) with_anonymous_union {
  int tag;
  union {
    int int_val;
    double double_val;
    tree tree_val;
  } GTY((desc("%0.tag"))) data;
};

/* Structure for testing array of pointers */
struct GTY(()) pointer_array_struct {
  tree * GTY((length("ptr_count"))) pointers;
  int ptr_count;
  struct plain_struct * GTY((length("struct_count"))) structs;
  int struct_count;
};

/* For testing callback in array context */
struct GTY(()) callback_container {
  walk_fn GTY((skip)) GTY((length("callback_count"))) callbacks;
  int callback_count;
};

#endif /* TEST_COVERAGE_H */
