/* test-coverage.h - Header file to test gengtype state generation coverage */
/* This file must be processed by gengtype to trigger all TYPE_* cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in some structures */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - triggers TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
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
  struct tree_list *GTY((skip)) next;  /* Self-referential pointer with skip */
  struct plain_struct *nested;  /* Pointer to another GTY-marked struct */
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Pointer to undefined type */
};

/* More pointer variations */
struct GTY(()) pointer_network {
  struct tree_list **GTY((skip)) list_array;  /* Pointer to pointer */
  void *GTY((skip)) raw_ptr;  /* Raw void pointer */
  const struct plain_struct *const_ptr;  /* Const pointer */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char ** GTY((length("str_count"))) string_array;
  int str_count;
};

/* Another array example with nested structures */
struct GTY(()) nested_array {
  struct plain_struct items[10];
  struct tree_list * GTY((length("list_len"))) lists;
  int list_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
/* Mimicking tree structure language-specific nodes */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  int block_number;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;  /* Explicit string tag */
  const char *description;  /* Implicit string */
  char * GTY((tag("STRING"))) mutable_str;  /* Mutable string */
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*walk_fn)(tree);
typedef int (*compare_fn)(const void *, const void *);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;  /* Function pointer with skip */
  compare_fn GTY((skip)) compare;
  void *GTY((skip)) user_data;
};

/* Complex structure combining multiple types */
struct GTY(()) comprehensive_example {
  /* Scalar types */
  my_scalar scalar_field;
  my_long_scalar long_field;
  enum color color_field;
  
  /* Struct types */
  struct plain_struct nested_struct;
  struct user_defined *user_struct_ptr;
  
  /* Union */
  union value_union data;
  
  /* Pointers */
  struct tree_list *list_head;
  struct comprehensive_example *GTY((skip)) next;  /* Self-reference */
  
  /* Arrays */
  int scores[3];
  tree * GTY((length("tree_count"))) tree_array;
  int tree_count;
  
  /* String */
  const char * GTY((tag("STRING"))) identifier;
  
  /* Callback */
  walk_fn GTY((skip)) traverse_fn;
  
  /* Language-specific */
  struct lang_specific_tree_node *lang_node;
};

/* Additional undefined type references to ensure TYPE_UNDEFINED is hit */
extern struct opaque_type *global_opaque_ptr;
typedef struct opaque_type *opaque_handle_t;

#endif /* TEST_COVERAGE_H */
