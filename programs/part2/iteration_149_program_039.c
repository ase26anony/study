/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_UNDEFINED: Forward declaration of opaque type */
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
  tree GTY((skip)) node;
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char * GTY((tag("STRING"))) s;
  double d;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;
  struct tree_list *prev;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_list *GTY((skip)) skip_ptr;
  struct pointer_network *self_ptr;  /* Self-referential */
  struct opaque_type *opaque_ptr;    /* Pointer to undefined type */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  const char * GTY((length("str_len"))) string_array;
  size_t str_len;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BINFO"))) another_lang_struct {
  tree base;
  tree * GTY((length("binfo_count"))) binfos;
  int binfo_count;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* Complex structure combining multiple types */
struct GTY(()) complex_type {
  /* Scalar fields */
  my_scalar id;
  color_enum status;
  
  /* Pointer fields */
  struct plain_struct *data;
  struct tree_list *list_head;
  
  /* Array field */
  struct named_object * GTY((length("obj_count"))) objects;
  int obj_count;
  
  /* Union field */
  union value_union current_value;
  
  /* String field */
  const char * GTY((tag("STRING"))) type_name;
  
  /* Callback field */
  walk_fn GTY((skip)) walker;
  
  /* Nested language structure */
  struct lang_specific_tree_node lang_node;
};

/* Another structure with nested arrays */
struct GTY(()) nested_arrays {
  struct array_container containers[3];
  struct pointer_network * GTY((length("net_count"))) networks;
  int net_count;
};

/* Union containing pointers */
union GTY(()) ptr_union {
  struct plain_struct *struct_ptr;
  struct tree_list *list_ptr;
  const char * GTY((tag("STRING"))) string_ptr;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
  int has_data;
  union ptr_union GTY((skip("!has_data"))) data;
  struct array_container * GTY((length("has_data ? container_count : 0"))) containers;
  int container_count;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
