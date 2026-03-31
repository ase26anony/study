/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Basic scalar types and typedefs */
typedef int my_scalar;
typedef long my_long;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;
  my_scalar count;
};

/* TYPE_USER_STRUCT: User-defined struct for special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: Union types */
union GTY(()) value_union {
  int i;
  const char *s;
  tree t;
};

struct GTY(()) union_container {
  int tag;
  union GTY((desc("tag"))) {
    int as_int;
    tree as_tree;
    const char *as_string;
  } GTY((tag("tag"))) value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;
  struct tree_list *prev;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct;
  struct tree_list *GTY((skip)) indirect;
  struct pointer_network *self_ref;
  struct user_defined *user_ptr;
};

/* TYPE_ARRAY: Arrays with different length annotations */
struct GTY(()) array_container {
  int fixed[5];
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  struct plain_struct GTY((length("struct_count"))) *struct_array;
  int struct_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_COMMON"))) another_lang_struct {
  tree common;
  unsigned int flags;
};

/* TYPE_STRING: String types */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

struct GTY(()) string_container {
  const char * GTY((tag("STRING"))) fixed_string;
  const char ** GTY((length("string_count"))) string_array;
  int string_count;
};

/* TYPE_CALLBACK: Struct with callback function pointer */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Composite structure using all types */
struct GTY(()) master_container {
  /* Scalar fields */
  my_scalar scalar_field;
  my_long long_field;
  enum { RED, GREEN, BLUE } color;
  
  /* Struct fields */
  struct plain_struct plain;
  struct user_defined *user;
  
  /* Union field */
  union value_union data;
  
  /* Pointer fields */
  struct tree_list *list;
  struct pointer_network *network;
  
  /* Array fields */
  struct array_container arrays;
  
  /* Language-specific struct */
  struct lang_specific_tree_node *lang_node;
  
  /* String fields */
  struct named_object named;
  struct string_container strings;
  
  /* Callback field */
  struct tree_walker walker;
  
  /* Undefined type reference */
  struct opaque_type *opaque_ref;
};

/* Additional pointer types for coverage */
typedef struct GTY(()) recursive_struct {
  int value;
  struct recursive_struct *left;
  struct recursive_struct *right;
} recursive_tree;

/* Array of pointers */
struct GTY(()) pointer_array_container {
  tree * GTY((length("ptr_count"))) pointers;
  int ptr_count;
  struct plain_struct ** GTY((length("struct_ptr_count"))) struct_ptrs;
  int struct_ptr_count;
};

/* Nested arrays */
struct GTY(()) nested_array {
  int matrix[3][3];
  tree * GTY((length("outer_len"))) *nested;
  int outer_len;
  int * GTY((length("inner_len"))) inner_lens;
};

#endif /* TEST_COVERAGE_H */
