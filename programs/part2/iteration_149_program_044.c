/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file should be processed by gengtype to trigger all TYPE_* cases */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in examples */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC marking */
  double value;
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
  double d;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct plain_struct *nested;  /* Pointer to another GTY struct */
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) child;
  struct recursive_node *sibling;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  unsigned list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  unsigned lang_flag;
  void * GTY((skip)) lang_data;
};

/* Another language-specific structure pattern */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree inheritance;
};

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long;
typedef unsigned int bitmask_t;

enum color { RED, GREEN, BLUE };

struct GTY(()) has_scalar {
  my_scalar count;
  my_long big_value;
  bitmask_t flags;
  enum color color;
  float float_val;
  double double_val;
};

/* TYPE_STRING: String fields with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  int id;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  compare_fn GTY((skip)) comparator;
  tree context;
};

/* More complex structure combining multiple types */
struct GTY(()) complex_type {
  /* Nested structures */
  struct plain_struct base;
  
  /* Pointers */
  struct tree_list *items;
  struct recursive_node *root;
  
  /* Arrays */
  struct array_container arrays;
  
  /* Union */
  union value_union data;
  
  /* String */
  const char * GTY((tag("STRING"))) tag;
  
  /* Scalar */
  my_scalar ref_count;
  
  /* Callback */
  walk_fn GTY((skip)) cleanup;
};

/* Chain of structures for testing traversal */
struct GTY(()) type_chain {
  struct GTY(()) chain_link {
    int id;
    struct chain_link *next;
    struct chain_link *prev;
  } *head;
  
  struct named_object *names;
  int count;
};

/* Test structure with conditional fields */
struct GTY(()) conditional_struct {
  int type;
  union {
    int int_val;
    double double_val;
    const char * GTY((tag("STRING"))) str_val;
  } GTY((desc("type"))) value;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_COVERAGE_H */
