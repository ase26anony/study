/* test-coverage.h - Header file to test gengtype state generation coverage */
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
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String type with tag */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  int id;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field for GC */
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_id;
  
  /* User-defined methods would be declared here in actual GCC */
  void (*cleanup)(struct user_defined *);
};

/* TYPE_UNION: Union within GTY-marked struct */
union GTY(()) value_union {
  int i;
  double d;
  const char * GTY((tag("STRING"))) s;
  tree t;
};

struct GTY(()) union_container {
  int tag;
  union value_union value;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct tree_list *GTY((chain_next("next"))) chain_next;
};

struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_list *list_head;
  struct user_defined * GTY((skip)) user_ptr;  /* Skip user-defined pointer */
  void * GTY((skip)) opaque_ptr;  /* Skip opaque pointer */
  
  /* Self-referential pointer */
  struct pointer_network *self_ptr;
  
  /* Mutual recursion */
  struct mutual_b *mutual;
};

struct GTY(()) mutual_b {
  int data;
  struct pointer_network *back_ref;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  
  /* Nested array in struct */
  struct plain_struct GTY((length("struct_count"))) *struct_array;
  int struct_count;
  
  /* Array of pointers */
  tree * GTY((length("ptr_count"))) *ptr_array;
  int ptr_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
/* Mimic tree structure with language-specific tags */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((skip)) context;
  unsigned int lang_flag : 1;
  
  /* Language-specific fields would go here */
  void * GTY((skip)) lang_specific;
};

/* Another language-specific structure pattern */
struct GTY((desc("%0.type"))) lang_struct_with_desc {
  tree type;
  int kind;
  
  union {
    struct plain_struct *plain;
    struct array_container *array;
    tree tree_val;
  } GTY((desc("kind == 1 ? 1 : kind == 2 ? 2 : 0"))) u;
};

/* Complex nested structure to test deep traversal */
struct GTY(()) nested_container {
  /* Direct types */
  struct plain_struct plain;
  struct array_container array;
  
  /* Pointers to various types */
  struct tree_list *list;
  struct union_container *union_ptr;
  
  /* Arrays of different types */
  struct plain_struct GTY((length("plain_count"))) *plain_array;
  int plain_count;
  
  tree GTY((length("tree_count"))) *tree_array;
  int tree_count;
  
  /* Callback field */
  walk_fn GTY((skip)) callback;
  
  /* String field */
  const char * GTY((tag("STRING"))) description;
  
  /* Scalar fields */
  my_scalar scalar_field;
  color_enum enum_field;
  
  /* Nested pointer to same type */
  struct nested_container *next;
};

/* Root structure that contains everything */
struct GTY(()) root_container {
  /* One of each major type category */
  struct plain_struct plain_instance;
  struct user_defined *user_instance;
  union value_union union_instance;
  struct tree_list *list_instance;
  struct array_container array_instance;
  struct lang_specific_tree_node lang_instance;
  struct nested_container nested_instance;
  
  /* Direct scalar fields */
  int scalar_int;
  my_scalar scalar_typedef;
  color_enum scalar_enum;
  
  /* String field */
  const char * GTY((tag("STRING"))) root_name;
  
  /* Callback field */
  transform_fn GTY((skip)) transformer;
  
  /* Pointer to undefined type */
  struct opaque_type * GTY((skip)) undefined_ptr;
};

#endif /* TEST_COVERAGE_H */
