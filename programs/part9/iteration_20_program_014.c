/* test-gtype.h - Comprehensive test of gengtype type system */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to trigger TYPE_UNDEFINED */
typedef struct forward_declared_struct *forward_ptr;
typedef union forward_declared_union *forward_union_ptr;

/* TYPE_SCALAR examples */
typedef int GTY(()) scalar_int;
typedef unsigned long GTY(()) scalar_ulong;

/* TYPE_STRING */
typedef const char *GTY(()) string_type;

/* TYPE_CALLBACK */
typedef void (*callback_func)(struct base_struct *);
typedef callback_func GTY((callback)) callback_type;

/* TYPE_STRUCT with various fields */
struct base_struct GTY(())
{
  scalar_int id;
  string_type name;
  struct base_struct *GTY((skip)) next_skip;  /* Skip from GC */
  struct base_struct *GTY((chain_next("%s.next"))) next_chain;
  forward_ptr forward_ref;  /* Will be TYPE_UNDEFINED initially */
};

/* TYPE_USER_STRUCT with custom marking */
struct user_struct GTY((user))
{
  void *custom_data;
  int user_tag;
};

/* TYPE_UNION with discriminator */
union variant_union GTY((desc("tag")))
{
  int tag;
  struct {
    int tag;
    scalar_int int_value;
  } GTY((tag("0"))) as_int;
  struct {
    int tag;
    string_type str_value;
    struct base_struct *GTY(()) ptr_value;
  } GTY((tag("1"))) as_complex;
};

/* TYPE_ARRAY examples */
struct array_container GTY(())
{
  /* Fixed-length array */
  struct base_struct *GTY(()) fixed_array[10];
  
  /* Variable-length array */
  struct user_struct *GTY((length("var_len"))) var_array;
  size_t var_len;
  
  /* Nested array */
  union variant_union *GTY((length("nested_len"))) *GTY((length("outer_len"))) nested_array;
  size_t outer_len;
  size_t *nested_len;
};

/* TYPE_POINTER variations */
typedef struct base_struct *GTY(()) base_ptr;
typedef union variant_union *GTY(()) variant_ptr;
typedef struct array_container *GTY(()) array_ptr;

/* Linked list using chain_next/chain_prev */
struct linked_list GTY(())
{
  scalar_int value;
  struct linked_list *GTY((chain_next("%s.next"), chain_prev("%s.prev"))) next;
  struct linked_list *prev;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void *GTY((skip)) data;
  size_t size;
};

/* Now define the forward-declared types to resolve TYPE_UNDEFINED */
struct forward_declared_struct GTY(())
{
  scalar_int magic;
  string_type description;
};

union forward_declared_union GTY(())
{
  scalar_int as_int;
  string_type as_string;
  struct base_struct *GTY(()) as_ptr;
};

/* TYPE_LANG_STRUCT - mimic GCC's tree structure */
#ifdef TEST_LANG_STRUCT
struct c_tree_node GTY(())
{
  int code;
  struct c_tree_node *GTY(()) operands[2];
  string_type identifier;
};
#endif

/* Complex nested structure to test deep traversal */
struct complex_nested GTY(())
{
  struct array_container container;
  union variant_union variant;
  callback_type callback;
  struct template_struct GTY((param_is(struct base_struct))) specialized;
};

#endif /* TEST_GTYPE_H */
