/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type */

/* Forward declarations for TYPE_UNDEFINED case */
struct GTY(()) opaque_type;  /* Never defined - triggers undefined type handling */

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
  tree GTY((skip)) node;  /* Skip this field during GC */
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
  const char * GTY((tag("STRING"))) s;
  double d;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Self-referential with skip */
  struct plain_struct *nested;  /* Pointer to another GTY struct */
};

struct GTY(()) pointer_network {
  struct tree_list *head;
  struct tree_list **GTY((skip)) tail_ptr;  /* Pointer to pointer with skip */
  void * GTY((atomic)) opaque_ptr;  /* Atomic pointer */
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  unsigned list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

struct GTY((tag("TS_BINFO"))) another_lang_struct {
  tree base;
  tree * GTY((length("binfo_count"))) binfo_array;
  unsigned binfo_count;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;  /* Explicit string tag */
  const char *description;  /* Implicit string */
  tree associated_tree;
};

/* Container struct that uses all types */
struct GTY(()) master_container {
  /* TYPE_SCALAR */
  my_scalar count;
  my_long_scalar big_count;
  color_enum current_color;
  
  /* TYPE_STRUCT */
  struct plain_struct plain;
  
  /* TYPE_USER_STRUCT */
  struct user_defined *user_data;
  
  /* TYPE_UNION */
  union value_union current_value;
  
  /* TYPE_POINTER */
  struct tree_list *items;
  struct pointer_network *network;
  
  /* TYPE_ARRAY */
  struct array_container arrays;
  
  /* TYPE_LANG_STRUCT */
  struct lang_specific_tree_node *lang_node;
  struct another_lang_struct *binfo;
  
  /* TYPE_STRING */
  struct named_object *named;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) walk_callback;
  transform_fn GTY((skip)) transform_callback;
  
  /* TYPE_POINTER to undefined type */
  struct opaque_type * GTY((skip)) opaque_ref;  /* Pointer to undefined type */
};

/* Additional complex types for edge cases */

/* Nested struct with union */
struct GTY(()) nested_example {
  struct GTY(()) inner_struct {
    int x;
    tree y;
  } inner;
  
  union GTY(()) {
    int as_int;
    tree as_tree;
  } data;
};

/* Chain of pointers forming a cycle */
struct GTY(()) cyclic_node {
  int id;
  struct cyclic_node * GTY((skip)) next;  /* Skip to avoid infinite recursion */
  struct cyclic_node *previous;  /* Regular pointer */
};

/* Array of pointers with nested length expression */
struct GTY(()) complex_array {
  tree * GTY((length("count * 2"))) double_array;
  struct tree_list ** GTY((length("list_count"))) list_ptrs;
  unsigned count;
  unsigned list_count;
};

/* Mixed array types */
struct GTY(()) mixed_arrays {
  int scalar_array[10];
  tree * GTY((length("tree_count"))) tree_array;
  const char * GTY((length("str_count"), tag("STRING"))) string_array;
  unsigned tree_count;
  unsigned str_count;
};

#endif /* TEST_COVERAGE_H */
