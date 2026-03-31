/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

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
  int user_id;
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
  struct plain_struct *related;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node **children;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  struct plain_struct * GTY((length("struct_count"))) struct_array;
  int struct_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
  struct GTY((tag("TS_COMMON"))) common_node *common;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree * GTY((length("vtable_size"))) vtable;
  int vtable_size;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree associated_tree;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

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
  struct tree_list *items;
  
  /* TYPE_ARRAY */
  struct named_object * GTY((length("name_count"))) names;
  int name_count;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) container_name;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) notify;
  
  /* TYPE_SCALAR */
  my_long_scalar timestamp;
  color_enum bg_color;
};

/* Additional pointer types for coverage */
struct GTY(()) pointer_network {
  /* Chain of pointers */
  struct pointer_network *next;
  struct pointer_network *prev;
  
  /* Array of pointers */
  struct plain_struct ** GTY((length("ptr_count"))) ptr_array;
  int ptr_count;
  
  /* Pointer to union */
  union value_union *current_value;
  
  /* Pointer to callback structure */
  struct tree_walker *walker;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct plain_struct *ps;
  struct tree_list *tl;
  struct complex_container *cc;
  void *generic;
};

/* Structure with nested anonymous union */
struct GTY(()) with_anonymous_union {
  int type;
  union {
    int int_val;
    tree tree_val;
    const char *str_val;
  } GTY((desc("type"))) value;
};

/* For testing TYPE_LANG_STRUCT with different tags */
struct GTY((tag("TS_VEC"))) lang_vector {
  tree * GTY((length("length"))) elements;
  int length;
  int capacity;
};

#endif /* TEST_COVERAGE_H */
