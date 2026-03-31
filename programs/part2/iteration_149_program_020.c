/* test-coverage.h - Comprehensive GTY type definitions for gengtype coverage */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* TYPE_UNDEFINED: Forward declaration of opaque type */
struct GTY(()) opaque_type;  /* Never defined, triggers undefined type handling */

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_scalar;
typedef long my_long_scalar;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRING: String type */
typedef const char *gcc_string;

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  my_scalar count;
  color_enum color;
};

/* TYPE_USER_STRUCT: User-defined type with special handling */
struct GTY((user)) user_defined {
  void *private_data;
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  double d;
  const char * GTY((tag("STRING"))) s;
  tree t;
};

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *GTY((chain_next("next"))) chain_next;  /* For chaining */
};

struct GTY(()) pointer_network {
  struct plain_struct *direct_ptr;
  struct tree_list *list_head;
  struct pointer_network *self_ref;  /* Self-referential pointer */
  void * GTY((skip)) opaque_ptr;  /* Opaque pointer to skip */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct plain_struct * GTY((length("struct_count"))) struct_array;
  int struct_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BLOCK"))) lang_block {
  tree vars;
  tree subblocks;
  int block_number;
};

/* Container struct using all types */
struct GTY(()) master_container {
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user;
  
  /* TYPE_UNION */
  union value_union value;
  
  /* TYPE_POINTER */
  struct pointer_network *network;
  struct tree_list *list;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  struct lang_block *block;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  my_long_scalar long_scalar;
  color_enum enum_field;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) name;
  gcc_string filename;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) walk_callback;
  transform_fn GTY((skip)) transform_callback;
  
  /* Reference to undefined type */
  struct opaque_type * GTY((skip)) opaque_ref;
};

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_data;
    tree inner_tree;
  } inner;
  
  union GTY(()) inner_union {
    int i;
    tree t;
  } data;
  
  struct outer_struct *next;
};

/* Array of pointers with nested length expression */
struct GTY(()) complex_array {
  struct plain_struct ** GTY((length("count * 2"))) ptr_array;
  int count;
  int multiplier;
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct plain_struct *struct_ptr;
  struct tree_list *list_ptr;
  void *generic_ptr;
};

#endif /* TEST_COVERAGE_H */
