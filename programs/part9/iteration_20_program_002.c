/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to trigger TYPE_UNDEFINED */
typedef struct forward_struct *forward_ptr;
typedef union forward_union *forward_union_ptr;

/* TYPE_SCALAR examples */
typedef int my_scalar_t;
typedef unsigned long my_ulong_t;

/* TYPE_STRING */
typedef const char *my_string_t;

/* TYPE_CALLBACK */
typedef void (*my_callback_t)(struct my_struct *);
typedef int (*compare_func_t)(const void *, const void *);

/* TYPE_STRUCT with various fields */
struct my_struct GTY(())
{
  int scalar_field;
  unsigned long ulong_field;
  const char *string_field GTY((skip));
  struct my_struct *next GTY((chain_next("%h.next")));
  forward_ptr forward_field;
  my_callback_t callback_field;
};

/* TYPE_USER_STRUCT with custom marking */
struct user_struct GTY((user))
{
  void *custom_data;
  int data_size;
};

/* TYPE_UNION with discriminator */
union my_union GTY((desc("tag")))
{
  int tag;
  struct {
    int type;
    void *data;
  } variant1;
  struct {
    long id;
    const char *name;
  } variant2;
};

/* TYPE_ARRAY examples */
struct array_container GTY(())
{
  int length;
  struct my_struct **elements GTY((length("%h.length")));
  
  int fixed_array[10];
  
  struct user_struct *user_array GTY((length("5")));
};

/* TYPE_POINTER variations */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef my_callback_t *callback_ptr;

/* Linked list structure for chain_next/prev testing */
struct linked_list GTY(())
{
  int value;
  struct linked_list *next GTY((chain_next("%h.next")));
  struct linked_list *prev GTY((chain_prev("%h.prev")));
};

/* Nested structure for complex traversal */
struct nested_container GTY(())
{
  struct my_struct inner_struct;
  union my_union inner_union;
  struct array_container *array_ptr;
  my_callback_t callbacks[5];
};

/* Now define the forward-declared types */
struct forward_struct GTY(())
{
  int defined_now;
  struct my_struct *link;
};

union forward_union GTY(())
{
  int int_val;
  struct forward_struct *struct_ptr;
};

/* Template-like structure with param_is */
struct template_struct GTY((param_is(T)))
{
  void *data;
  int (*compare)(const void *, const void *);
};

/* Language-specific structure (simulating c_tree_node) */
#ifdef TEST_LANG_STRUCT
struct c_tree_node GTY(())
{
  int code;
  struct c_tree_node *kids[2];
  union tree_union *u;
};

union tree_union GTY((desc("code")))
{
  int code;
  struct {
    const char *name;
    int value;
  } decl;
  struct {
    int op;
    struct c_tree_node *operands[2];
  } expr;
};
#endif

#endif /* TEST_GTYPE_H */
