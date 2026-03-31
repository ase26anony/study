/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
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

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;
  my_scalar count;
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
  tree t;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;
  struct tree_list *prev;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct;
  struct tree_list *GTY((chain_next("next"))) chain;
  struct user_defined *user_ptr;
  struct pointer_network *self_ref;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_buffer;
  int str_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_COMMON"))) common_tree_node {
  tree chain;
  tree type;
  enum tree_code code : 8;
};

/* TYPE_STRING: String type with tag attribute */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("IDENTIFIER"))) identifier;
};

/* TYPE_CALLBACK: Struct with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER network */
  struct pointer_network *network;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT pointer */
  struct lang_specific_tree_node *lang_node;
  
  /* TYPE_STRING */
  struct named_object naming;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  enum color color_field;
  
  /* Forward reference to TYPE_UNDEFINED */
  struct opaque_type *GTY((skip)) opaque_ref;
};

/* Another union type with nested structures */
union GTY(()) nested_union {
  struct plain_struct ps;
  struct array_container ac;
  struct named_object no;
};

/* Self-referential structure with arrays */
struct GTY(()) recursive_array {
  int depth;
  struct recursive_array * GTY((length("depth"))) children;
  tree * GTY((length("depth * 2"))) data;
};

/* Structure with conditional pointers */
struct GTY(()) conditional_struct {
  int has_extra;
  tree GTY((skip("!has_extra"))) extra_data;
  struct plain_struct * GTY((skip("!has_extra"))) extra_struct;
};

#endif /* TEST_COVERAGE_H */
