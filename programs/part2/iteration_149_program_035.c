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
  tree t;
};

/* TYPE_ARRAY: Structs with various array types */
struct GTY(()) array_container {
  /* Fixed-size array */
  int fixed[5];
  
  /* Variable-length array with length attribute */
  tree * GTY((length("dynamic_count"))) var_array;
  int dynamic_count;
  
  /* Nested array in struct */
  struct GTY(()) nested {
    char data[10];
  } nested_array[3];
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer in GC */
  struct tree_list *GTY((chain_next("next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) recursive_node {
  int id;
  struct recursive_node *GTY((skip)) parent;
  struct recursive_node *GTY((length("child_count"))) children;
  int child_count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  tree GTY((tag("TS_COMMON"))) common;
  unsigned int lang_specific_flags;
};

/* Another language-specific pattern */
struct GTY((tag("TS_BINFO"))) lang_binfo {
  tree base;
  tree GTY((length("vtable_count"))) vtables;
  int vtable_count;
};

/* TYPE_STRING: String type handling */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  const char * GTY((tag("STRING"))) description;
  tree associated_tree;
};

/* TYPE_CALLBACK in struct context */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) pre_order_callback;
  walk_fn GTY((skip)) post_order_callback;
  transform_fn GTY((skip)) transform_callback;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct GTY(()) header {
    int version;
    const char * GTY((tag("STRING"))) name;
  } header;
  
  /* TYPE_UNION */
  union GTY(()) data {
    int int_val;
    double double_val;
    tree tree_val;
    struct plain_struct *struct_ptr;
  } data;
  
  /* TYPE_ARRAY of pointers */
  struct tree_list * GTY((length("list_count"))) lists[10];
  int list_count;
  
  /* TYPE_POINTER network */
  struct recursive_node *GTY((skip)) root_node;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) notify_callback;
};

/* Template-like structure for edge cases */
struct GTY(()) template_struct {
  /* Multiple scalar types */
  my_scalar scalar1;
  my_long_scalar scalar2;
  color_enum color;
  
  /* Optional pointer with conditional */
  struct plain_struct * GTY((skip("is_null"))) optional_ptr;
  int is_null;
  
  /* Union with discriminant */
  union GTY(()) discriminated {
    int as_int;
    tree as_tree;
    const char * GTY((tag("STRING"))) as_string;
  } value;
  int discriminant;  /* 0=int, 1=tree, 2=string */
};

/* Structure with all basic types */
struct GTY(()) all_types {
  /* TYPE_SCALAR */
  int integer;
  long long_integer;
  unsigned int unsigned_integer;
  color_enum enum_value;
  
  /* TYPE_STRING */
  const char * GTY((tag("STRING"))) string_field;
  
  /* TYPE_POINTER */
  struct plain_struct *struct_pointer;
  struct tree_list *list_pointer;
  
  /* TYPE_ARRAY */
  int int_array[8];
  tree tree_array[4];
  
  /* TYPE_UNION */
  union value_union union_field;
  
  /* TYPE_CALLBACK */
  walk_fn GTY((skip)) callback_field;
  
  /* Reference to user-defined type */
  struct user_defined * GTY((skip)) user_data;
};

/* For testing parameterized types */
typedef struct GTY(()) param_struct<int N> {
  int data[N];
  tree extra;
} sized_struct;

#endif /* TEST_COVERAGE_H */
