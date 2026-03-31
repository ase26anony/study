/* test-coverage.h - Comprehensive GTY type definitions for coverage testing */
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
typedef long my_long;
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
  const char *s;
  tree t;
};

/* TYPE_POINTER: Complex pointer networks */
struct GTY(()) tree_list {
  tree value;
  struct tree_list *GTY((skip)) next;  /* Skip pointer */
  struct tree_list *GTY((chain_next ("next"))) chain_next;
};

/* Self-referential structure */
struct GTY(()) recursive_struct {
  int id;
  struct recursive_struct *GTY((skip)) parent;
  struct recursive_struct *GTY((child)) children;
};

/* TYPE_ARRAY: Arrays with different GTY length annotations */
struct GTY(()) array_container {
  int fixed[5];  /* Fixed-size array */
  tree * GTY((length("dynamic_count"))) var_array;  /* Variable-length array */
  int dynamic_count;
  struct tree_list * GTY((length("list_count"))) list_array;
  size_t list_count;
};

/* TYPE_LANG_STRUCT: Language-specific frontend structure */
struct GTY((tag("TS_TYPED"))) lang_specific_tree_node {
  tree type;
  int lang_specific_data;
};

/* Another language-specific structure */
struct GTY((tag("TS_BINFO"))) base_binfo {
  tree base;
  int offset;
};

/* TYPE_STRING usage in struct */
struct GTY(()) named_object {
  const char * GTY((tag("STRING"))) name;
  tree decl;
  my_scalar priority;
};

/* TYPE_CALLBACK usage in struct */
struct GTY(()) tree_walker {
  walk_fn GTY((skip)) callback;
  transform_fn GTY((skip)) transformer;
  void * GTY((skip)) user_data;
};

/* Complex nested structure combining multiple types */
struct GTY(()) complex_container {
  /* TYPE_STRUCT nested */
  struct GTY(()) inner_struct {
    int x;
    tree y;
  } inner;
  
  /* TYPE_UNION */
  union GTY(()) data_union {
    int int_val;
    double double_val;
    tree tree_val;
  } data;
  
  /* TYPE_ARRAY of pointers */
  struct tree_list ** GTY((length("ptr_count"))) ptr_array;
  int ptr_count;
  
  /* TYPE_STRING array */
  const char * GTY((tag("STRING"), length("name_count"))) *names;
  int name_count;
  
  /* TYPE_CALLBACK array */
  walk_fn GTY((skip)) *callbacks;
  int callback_count;
};

/* Structure with pointer to undefined type */
struct GTY(()) uses_undefined {
  struct opaque_type *GTY((skip)) opaque_ptr;
  int known_data;
};

/* Template-like structure for C++ frontend simulation */
struct GTY(()) template_decl {
  tree template;
  tree * GTY((length("parm_count"))) template_parms;
  int parm_count;
  const char * GTY((tag("STRING"))) template_name;
};

#endif /* TEST_COVERAGE_H */
