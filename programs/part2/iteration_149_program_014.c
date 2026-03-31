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

enum gty_test_enum {
  GTY_ENUM_A,
  GTY_ENUM_B,
  GTY_ENUM_C
};

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
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
  int id;
  struct self_ref *GTY((skip)) next;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  int list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree vtable;
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
  /* Scalar fields */
  my_scalar scalar_field;
  my_long_scalar long_field;
  enum gty_test_enum enum_field;
  
  /* Struct field */
  struct plain_struct nested_struct;
  
  /* Union field */
  union value_union data;
  
  /* Pointer fields */
  struct tree_list *list_head;
  struct self_ref *self_chain;
  
  /* Array field */
  struct array_container arrays;
  
  /* String field */
  const char * GTY((tag("STRING"))) type_name;
  
  /* Callback field */
  walk_fn GTY((skip)) walker;
  
  /* Reference to undefined type (should trigger TYPE_UNDEFINED) */
  struct opaque_type *GTY((skip)) opaque_ref;
};

/* Container structure with arrays of different types */
struct GTY(()) type_container {
  /* Array of structs */
  struct plain_struct GTY((length("struct_count"))) *struct_array;
  int struct_count;
  
  /* Array of pointers */
  tree * GTY((length("tree_count"))) tree_array;
  int tree_count;
  
  /* Array of strings */
  const char * GTY((length("name_count"), tag("STRING"))) *name_array;
  int name_count;
  
  /* Multi-dimensional array */
  int matrix[3][3];
};

/* Union containing pointers */
union GTY(()) ptr_union {
  tree tree_ptr;
  struct plain_struct *struct_ptr;
  const char *string_ptr;
};

/* Nested structure with all type combinations */
struct GTY(()) master_container {
  /* Direct containment */
  struct plain_struct direct_struct;
  union value_union direct_union;
  
  /* Pointer containment */
  struct tree_list *indirect_list;
  struct complex_type *complex_ptr;
  
  /* Array containment */
  struct array_container direct_array;
  struct type_container *container_ptr;
  
  /* Language-specific */
  struct lang_specific_tree_node lang_node;
  
  /* String */
  struct named_object named;
  
  /* Callback */
  struct tree_walker walker;
  
  /* Scalar */
  my_scalar master_scalar;
  
  /* Reference to undefined */
  struct opaque_type *GTY((skip)) undefined_ref;
};

#endif /* TEST_COVERAGE_H */
