/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations */
struct GTY(()) opaque_type;  /* TYPE_UNDEFINED - forward declaration */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;                    /* Scalar field */
  tree GTY((skip)) node;    /* Pointer with skip attribute */
  enum color c;             /* Enum scalar */
};

/* TYPE_USER_STRUCT: User-defined type handling */
struct GTY((user)) user_defined {
  void *private_data;       /* Opaque pointer */
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
  struct tree_list *GTY((skip)) next;  /* Self-referential pointer */
  struct plain_struct *related;         /* Pointer to another GTY struct */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];                         /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list ** GTY((length("list_count"))) list_array;
  size_t list_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure pattern */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree inherits;
};

/* TYPE_STRING: String type */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;  /* String field */
  int id;
};

/* TYPE_CALLBACK in struct context */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;        /* Function pointer */
  transform_fn GTY((skip)) transformer; /* Another function pointer */
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct plain_struct base;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER network */
  struct tree_list *GTY((skip)) head;
  struct named_object *names;
  
  /* TYPE_ARRAY */
  tree GTY((length("num_children"))) children[10];  /* Embedded array */
  int num_children;
  
  /* TYPE_SCALAR */
  my_scalar count;
  my_long_scalar big_count;
  enum color default_color;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) description;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) visitor;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_defined *user_data;
};

/* Another union type with nested structures */
union GTY(()) nested_union {
  struct plain_struct ps;
  struct array_container ac;
  const char * GTY((tag("STRING"))) str;
};

/* Chain structure for testing pointer traversal */
struct GTY(()) type_chain {
  int id;
  struct type_chain *GTY((skip)) next;
  struct type_chain *GTY((skip)) prev;
  union nested_union data;
};

/* Global variable declarations for gengtype to process */
extern struct plain_struct * GTY((root)) global_plain_struct;
extern struct complex_container * GTY((root)) global_complex;
extern union value_union GTY((root)) global_union;

#endif /* TEST_COVERAGE_H */
