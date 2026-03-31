/* test-coverage.h - Header file to test gengtype state generation coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* This will be TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned int my_unsigned_scalar;

enum color { RED, GREEN, BLUE };

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

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct plain_struct *nested;  /* Pointer to another GTY struct */
};

/* Self-referential structure */
struct GTY(()) self_ref {
  int data;
  struct self_ref *GTY((chain_next("%h.next"))) next;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_array;
  int str_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_COMMON"))) lang_common_node {
  tree chain;
  tree type;
  int code;
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

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct plain_struct nested_struct;
  
  /* TYPE_UNION */
  union value_union current_value;
  
  /* TYPE_POINTER network */
  struct tree_list *items;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  enum color color_field;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) identifier;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) notify_callback;
  
  /* Pointer to undefined type (TYPE_UNDEFINED) */
  struct opaque_type *GTY((skip)) opaque_ref;
};

/* Union containing structs */
union GTY(()) type_union {
  struct plain_struct as_struct;
  struct tree_list *as_list;
  struct array_container as_array;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
  int has_children;
  union {
    struct tree_list *GTY((tag("0"))) single_child;
    struct tree_list **GTY((tag("1"))) children;
  } GTY((desc("%1.has_children"))) u;
  int child_count;
};

#endif /* TEST_COVERAGE_H */
