/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"  /* For tree type used in examples */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_type;

/* TYPE_SCALAR: Fundamental scalar types and typedefs */
typedef int my_scalar;
typedef long my_long_scalar;
enum color { RED, GREEN, BLUE };

/* TYPE_CALLBACK: Function pointer type */
typedef void (*walk_fn)(tree);
typedef tree (*transform_fn)(tree, void*);

/* TYPE_STRING: String type definition */
typedef const char *gcc_string;

/* TYPE_STRUCT: Plain C struct with GTY markers */
struct GTY(()) plain_struct {
  int a;
  tree GTY((skip)) node;  /* Skip this field during GC */
  enum color color;
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

/* TYPE_ARRAY: Structs with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  
  /* Nested array in struct */
  struct GTY(()) nested {
    char data[10];
  } nested_array[3];
  
  int dynamic_count;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list * GTY((skip)) next;  /* Skip pointer for linked list */
  struct tree_list * GTY((chain_next("next"))) chain;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node * GTY((skip)) parent;
  struct recursive_node * GTY((length("child_count"))) children;
  int child_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree inheritance;
};

/* TYPE_STRING usage in struct */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  tree decl;
  my_scalar priority;
};

/* TYPE_CALLBACK usage in struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void * GTY((skip)) user_data;
};

/* Combined structure using multiple type kinds */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct GTY(()) header {
    int version;
    gcc_string GTY((tag("STRING"))) description;
  } header;
  
  /* TYPE_UNION */
  union GTY(()) data {
    int int_val;
    tree tree_val;
    struct plain_struct * GTY((skip)) struct_ptr;
  } data;
  
  /* TYPE_ARRAY of pointers */
  tree * GTY((length("num_elements"))) elements;
  
  /* TYPE_POINTER to callback structure */
  struct tree_walker * GTY((skip)) walker;
  
  /* TYPE_SCALAR */
  my_scalar count;
  my_long_scalar total_size;
  enum color background;
  
  int num_elements;
};

/* Template-like structure for additional coverage */
struct GTY(()) template_instance {
  struct complex_container * GTY((skip)) instance;
  struct template_instance * GTY((skip)) next_template;
};

/* Extern declaration to satisfy possible references */
extern struct opaque_type * GTY((skip)) global_opaque;

#endif /* TEST_COVERAGE_H */
