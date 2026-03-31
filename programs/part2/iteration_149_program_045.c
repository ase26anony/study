/* test-coverage.h - Header file to test gengtype state generation coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;

enum color { RED, GREEN, BLUE };

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
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

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *GTY((chain_next ("%h.next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *GTY((child)) children;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char ** GTY((length("string_count"))) string_array;
  int string_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure pattern */
struct GTY((tag("TS_BINFO"))) binfo_struct {
  tree base;
  tree inheritance;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) complex_type {
  /* Nested structures */
  struct plain_struct base;
  
  /* Union field */
  union value_union data;
  
  /* Arrays */
  struct array_container arrays;
  
  /* Pointers */
  struct tree_list *GTY((skip)) list_head;
  struct recursive_node *GTY((child)) tree_root;
  
  /* Strings */
  struct named_object name_info;
  
  /* Callback */
  struct tree_walker walker;
  
  /* Scalar types */
  my_long_scalar big_number;
  enum color default_color;
};

/* Container structure to ensure all types are referenced */
struct GTY(()) type_container {
  struct plain_struct *plain;
  struct user_defined *user;
  union value_union *union_ptr;
  struct tree_list *list;
  struct recursive_node *recursive;
  struct array_container *arrays;
  struct lang_specific_tree_node *lang;
  struct binfo_struct *binfo;
  struct named_object *named;
  struct tree_walker *walker;
  struct complex_type *complex;
  
  /* Undefined type pointer */
  struct opaque_type *GTY((skip)) opaque_ptr;
};

#endif /* TEST_COVERAGE_H */
