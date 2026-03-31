/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#ifdef GENERATOR_FILE
#include "ansidecl.h"
#include "system.h"
#endif

/* Forward declarations to test TYPE_UNDEFINED */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(void *data, int value);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_STRUCT with various fields */
struct my_base_struct GTY(())
{
  my_scalar_t id;
  my_string_t name;
  struct my_base_struct *GTY((skip)) next_skipped;
  struct my_base_struct *GTY((chain_next("%h.next"))) next;
  struct my_base_struct *GTY((chain_prev("%h.prev"))) prev;
};

/* TYPE_USER_STRUCT with custom marking */
struct my_user_struct GTY((user))
{
  int data;
  void *private_ptr;
};

/* TYPE_UNION */
union my_union GTY(())
{
  int as_int;
  my_string_t as_string;
  struct my_base_struct *GTY((tag("1"))) as_struct;
  void *as_ptr;
};

/* Discriminated union with desc */
struct tagged_union GTY(())
{
  int tag;
  union my_union GTY((desc("%0.tag"))) value;
};

/* TYPE_ARRAY - variable length */
struct array_container GTY(())
{
  int count;
  struct my_base_struct * GTY((length("%h.count"))) items[1];
};

/* TYPE_ARRAY - fixed length */
struct fixed_array GTY(())
{
  struct my_base_struct *items[10];
  my_scalar_t numbers[5];
};

/* TYPE_POINTER variations */
typedef struct my_base_struct *base_ptr_t;
typedef union my_union *union_ptr_t;
typedef forward_ptr undefined_ptr_t;

/* Linked list structure */
struct linked_list GTY(())
{
  int value;
  struct linked_list *GTY((chain_next("%h.next"))) next;
};

/* Nested structure with array of pointers */
struct complex_struct GTY(())
{
  struct my_base_struct header;
  union my_union data;
  struct complex_struct *GTY((skip)) parent;
  struct linked_list *children;
  int child_count;
  struct linked_list * GTY((length("%h.child_count"))) child_array[1];
};

/* Structure with callback field */
struct callback_container GTY(())
{
  my_callback_t callback;
  void *user_data;
  compare_func_t compare;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void *data;
  int size;
};

/* Structure using maybe_undef */
struct uses_undefined GTY(())
{
  forward_ptr GTY((maybe_undef)) maybe_defined;
  struct forward_declared_struct *GTY((maybe_undef)) direct_ptr;
};

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GENERATOR_FILE
struct c_tree_node;
typedef struct c_tree_node *tree;
#endif

struct lang_specific GTY(())
{
#ifdef GENERATOR_FILE
  tree decl;
#endif
  int lang_specific_data;
};

/* Now define the forward-declared structure */
struct forward_declared_struct GTY(())
{
  int finally_defined;
  struct my_base_struct *related;
};

#endif /* TEST_GTYPE_H */
