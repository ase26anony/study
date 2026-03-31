/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */
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
  struct tree_list *prev;  /* Regular GTY pointer */
};

/* Self-referential structure */
struct GTY(()) linked_node {
  int id;
  struct linked_node *GTY((skip)) self_ptr;
  struct linked_node *children[3];
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  const char * GTY((length("str_len"))) strings;
  size_t str_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) binfo_struct {
  tree base;
  tree * GTY((length("vtable_size"))) vtable;
  int vtable_size;
};

/* TYPE_STRING: String fields with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(tree, tree);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) comparator;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct GTY(()) inner_struct {
    int x;
    tree y;
  } inner;
  
  /* TYPE_UNION */
  union GTY(()) data_union {
    int int_val;
    double double_val;
    tree tree_val;
  } data;
  
  /* TYPE_ARRAY of pointers */
  struct tree_list * GTY((length("list_count"))) lists;
  int list_count;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) identifier;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) traverse_fn;
  
  /* TYPE_POINTER to user-defined type */
  struct user_defined * GTY((skip)) user_data;
  
  /* TYPE_POINTER to undefined type */
  struct opaque_type * GTY((skip)) opaque_ref;
};

/* TYPE_UNION in a struct context */
struct GTY(()) union_container {
  enum color color;
  union {
    int int_value;
    tree tree_value;
    struct plain_struct *struct_ptr;
  } GTY((desc("color"))) value;
};

/* Array of unions */
struct GTY(()) union_array {
  union value_union GTY((length("union_count"))) unions;
  int union_count;
};

/* Chain of structures with various pointer types */
struct GTY(()) type_chain {
  struct plain_struct *plain;
  struct user_defined *user;
  struct array_container *array;
  struct lang_specific_tree_node *lang;
  struct named_object *named;
  struct tree_walker *walker;
  struct complex_container *complex;
  struct union_container *union_cont;
  struct union_array *union_arr;
  
  /* Self-reference */
  struct type_chain *next;
};

#endif /* TEST_COVERAGE_H */
