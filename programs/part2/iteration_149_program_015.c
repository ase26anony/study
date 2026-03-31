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
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during marking */
  my_scalar count;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;  /* User handles marking manually */
  int user_id;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  tree t;
};

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *GTY((chain_next("next"))) chain_next;
};

struct GTY(()) pointer_container {
  struct plain_struct *direct_ptr;
  struct tree_list *list_head;
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Pointer to undefined type */
  void *GTY((skip)) raw_ptr;
};

/* TYPE_ARRAY: Arrays with different GTY annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  const char * GTY((length("str_len + 1"))) string_array;
  int str_len;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree inheritance_chain;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* TYPE_CALLBACK in a struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void *GTY((skip)) user_data;
};

/* Complex nested structure to test multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT member */
  struct plain_struct plain;
  
  /* TYPE_UNION member */
  union value_union value;
  
  /* TYPE_ARRAY members */
  struct array_container arrays;
  
  /* TYPE_POINTER network */
  struct pointer_container *pointers;
  
  /* TYPE_STRING members */
  struct named_object naming;
  
  /* TYPE_SCALAR members */
  my_scalar scalar_field;
  color_enum color;
  
  /* TYPE_CALLBACK member */
  struct tree_walker walker;
  
  /* Pointer to TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  
  /* Pointer to TYPE_USER_STRUCT */
  struct user_defined *user_data;
};

/* Additional pointer types for coverage */
typedef struct GTY(()) recursive_struct {
  int data;
  struct recursive_struct *next;
} recursive_struct_t;

/* Array of pointers */
struct GTY(()) pointer_array {
  tree * GTY((length("ptr_count"))) pointers;
  int ptr_count;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  tree tree_ptr;
  struct plain_struct *struct_ptr;
  const char *string_ptr;
};

#endif /* TEST_COVERAGE_H */
