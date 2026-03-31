/* test-gtype.h - Comprehensive test of gengtype type categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t GTY(());
typedef unsigned long my_ulong_t GTY(());

/* TYPE_STRING */
typedef const char *my_string_t GTY(());

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(struct my_struct *s, int value) GTY((callback));

/* TYPE_USER_STRUCT */
struct my_user_struct GTY((user))
{
  int data;
  void (*marker)(void *);
};

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  /* Scalar fields */
  my_scalar_t id;
  my_ulong_t timestamp;
  
  /* Pointer fields (GC roots) */
  struct my_struct *next GTY((skip));  /* Skip from GC */
  struct my_struct *prev GTY((chain_prev("next")));
  
  /* String field */
  my_string_t name;
  
  /* Array field */
  struct my_struct **children GTY((length("child_count")));
  int child_count;
  
  /* Union field */
  union my_union *variant GTY((desc("type_tag")));
  
  /* Callback field */
  my_callback_t callback;
  
  /* Forward declared pointer (may be TYPE_UNDEFINED) */
  forward_ptr forward GTY((maybe_undef));
  
  /* Tag for discriminated union */
  int type_tag;
};

/* TYPE_UNION */
union my_union GTY((desc("type_tag")))
{
  int int_value;
  my_string_t string_value;
  struct my_struct *struct_ptr;
  double double_value;
};

/* TYPE_ARRAY variations */
struct array_container GTY(())
{
  /* Fixed-length array */
  int fixed_array[10] GTY(());
  
  /* Variable-length array */
  struct my_struct **var_array GTY((length("var_count")));
  int var_count;
  
  /* Nested array */
  int **matrix GTY((length("rows"), length("cols[?]")));
  int rows;
  int *cols GTY((length("rows")));
};

/* TYPE_POINTER variations */
typedef struct my_struct *my_struct_ptr GTY(());
typedef union my_union *my_union_ptr GTY(());
typedef my_callback_t *callback_ptr GTY(());

/* Linked list structure for chain_next testing */
struct linked_list GTY(())
{
  int value;
  struct linked_list *next GTY((chain_next));
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void *data;
  int (*compare)(void *, void *);
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
struct c_tree_node GTY(())
{
  int code;
  struct c_tree_node *kids[2];
  my_string_t name;
};
#endif

/* Nested structure for complex graphs */
struct outer_struct GTY(())
{
  struct my_struct inner;
  struct array_container container;
  union my_union variants[5];
  struct outer_struct *recursive_ptr;
};

#endif /* TEST_GTYPE_H */
