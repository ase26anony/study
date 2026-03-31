/* test-coverage.h - Header file to test gengtype state generation coverage */
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
typedef long my_long_scalar;

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
  int user_tag;
};

/* TYPE_UNION: GTY-marked union */
union GTY(()) value_union {
  int i;
  const char *s;
  double d;
};

/* TYPE_POINTER: Complex pointer network */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer for linked list */
  struct plain_struct *nested;
};

/* Self-referential structure */
struct GTY(()) recursive_struct {
  int id;
  struct recursive_struct *GTY((skip)) child;
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
  location_t locus;
};

/* TYPE_STRING: String type with tag attribute */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void *GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct plain_struct nested_struct;
  
  /* TYPE_UNION */
  union value_union data;
  
  /* TYPE_POINTER network */
  struct tree_list *GTY((skip)) items;
  
  /* TYPE_ARRAY */
  struct array_container GTY((skip)) arrays[3];
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) identifier;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) callback;
  
  /* TYPE_SCALAR */
  my_scalar scalar_field;
  enum color color_field;
};

/* Structure with pointer to undefined type (TYPE_UNDEFINED) */
struct GTY(()) uses_undefined {
  int valid;
  struct opaque_type *GTY((skip)) opaque_ptr;  /* Points to undefined type */
};

/* Union within struct */
struct GTY(()) struct_with_union {
  int tag;
  union {
    int int_val;
    const char *str_val;
    tree tree_val;
  } GTY((desc("tag"))) value;
};

/* Array of pointers */
struct GTY(()) pointer_array_container {
  tree * GTY((length("ptr_count"))) pointers;
  int ptr_count;
  struct plain_struct * GTY((length("struct_count"))) struct_ptrs;
  int struct_count;
};

#endif /* TEST_COVERAGE_H */
