/* test-coverage.h - Header file to test gengtype state generation coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for pointer types */
struct GTY(()) plain_struct;
struct GTY(()) array_container;
struct GTY(()) tree_walker;

/* TYPE_UNDEFINED: Forward declaration of opaque type that's never defined */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  enum color color;
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
  const char * GTY((tag("STRING"))) s;
  tree t;
};

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct plain_struct *nested;
  struct array_container *container;
  struct tree_walker *walker;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *children[4];  /* Fixed array of pointers */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_buffer;
  int str_len;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure pattern */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree inheritance_chain;
};

/* TYPE_STRING: String type with STRING tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) filename;
};

/* TYPE_CALLBACK: Structure with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Container structure that uses all types */
struct GTY(()) type_container {
  /* TYPE_STRUCT */
  struct plain_struct plain GTY((skip));
  
  /* TYPE_USER_STRUCT */
  struct user_defined user;
  
  /* TYPE_UNION */
  union value_union value;
  
  /* TYPE_POINTER */
  struct tree_list *list;
  struct recursive_node *root;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node lang_node;
  struct lang_binfo *binfo;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  my_long long_field;
  enum color color_field;
  
  /* TYPE_STRING */
  struct named_object named;
  
  /* TYPE_CALLBACK */
  struct tree_walker walker;
  
  /* TYPE_UNDEFINED (pointer to undefined type) */
  struct opaque_type *opaque_ptr;
};

/* Additional pointer variations */
struct GTY(()) pointer_network {
  /* Chain of pointers */
  struct pointer_network *next;
  struct pointer_network *prev GTY((skip));
  
  /* Array of pointers */
  struct plain_struct *struct_array[10];
  
  /* Pointer to array */
  int *int_array_ptr;
  
  /* Pointer to pointer */
  tree *tree_ptr_ptr;
  
  /* Conditional pointer */
  struct array_container * GTY((skip("condition"))) conditional_ptr;
  int condition;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct plain_struct *struct_ptr;
  struct array_container *array_ptr;
  const char *string_ptr;
};

#endif /* TEST_COVERAGE_H */
