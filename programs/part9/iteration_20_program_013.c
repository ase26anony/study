/* test-gtype.h - Test file for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to trigger TYPE_UNDEFINED */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_other_scalar_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(void *data, int value);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_STRUCT with various fields */
struct GTY(()) my_base_struct {
  int GTY((skip)) private_field;  /* Not traced by GC */
  my_scalar_t scalar_field;
  my_string_t string_field;
  struct my_base_struct *GTY((chain_next("%s.next"), chain_prev("%s.prev"))) next;
  struct my_base_struct *prev;
};

/* TYPE_UNION */
union GTY((desc("type_tag"))) my_variant_union {
  int GTY((tag("0"))) int_val;
  double GTY((tag("1"))) double_val;
  my_string_t GTY((tag("2"))) string_val;
  struct my_base_struct *GTY((tag("3"))) struct_ptr;
  int type_tag;  /* Discriminant */
};

/* TYPE_USER_STRUCT with custom marking */
struct GTY((user)) my_user_struct {
  void *custom_data;
  size_t data_size;
  /* User-defined marking function will be provided */
};

/* TYPE_ARRAY - variable length */
struct GTY(()) my_array_container {
  int count;
  struct my_base_struct * GTY((length("%h.count"))) items[];
};

/* TYPE_ARRAY - fixed length */
struct GTY(()) my_fixed_array {
  struct my_base_struct * GTY((length("10"))) fixed_items[10];
  union my_variant_union GTY((length("5"))) variants[5];
};

/* TYPE_POINTER in various forms */
typedef struct my_base_struct *base_ptr_t;
typedef union my_variant_union *variant_ptr_t;

/* Complex nested structure */
struct GTY(()) my_complex_struct {
  struct my_base_struct base;
  union my_variant_union current_variant;
  struct my_array_container *dynamic_array;
  struct my_fixed_array fixed_data;
  my_callback_t callback;
  forward_ptr undefined_ptr;  /* Will be TYPE_UNDEFINED initially */
};

/* Linked list structure */
struct GTY(()) my_list_node {
  int id;
  my_string_t name;
  struct my_list_node *GTY((skip)) skip_ptr;  /* Not traced */
  struct my_list_node *GTY((chain_next("%s.next"))) next;
};

/* Template-like structure with param_is */
struct GTY((param_is(T))) my_template {
  T *data;
  int size;
};

/* TYPE_LANG_STRUCT - mimic GCC's tree structure */
#ifdef TEST_LANG_STRUCT
struct GTY((tag("TS_BASE"))) c_tree_base {
  enum tree_code code : 16;
  unsigned side_effects_flag : 1;
  unsigned constant_flag : 1;
  unsigned addressable_flag : 1;
};

struct GTY((desc("TREE_CODE((tree)&%h)"))) c_tree_node {
  struct c_tree_base base;
  union {
    struct my_base_struct *GTY((tag("0"))) as_struct;
    my_string_t GTY((tag("1"))) as_string;
    int GTY((tag("2"))) as_int;
  } GTY((desc("%h.base.code"))) u;
};
#endif

/* Another forward declaration */
struct GTY(()) incomplete_struct;

#endif /* TEST_GTYPE_H */
