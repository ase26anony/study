/* test-gtype.h - Comprehensive test for gengtype type coverage */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to test TYPE_UNDEFINED */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR - Basic scalar types */
typedef int GTY(()) scalar_int;
typedef unsigned long GTY(()) scalar_ulong;
typedef double GTY(()) scalar_double;

/* TYPE_STRING */
typedef const char *GTY(()) string_type;

/* TYPE_CALLBACK */
typedef void (*callback_func)(void *data);
typedef void (*GTY((callback)) complex_callback)(struct test_struct *arg1, int arg2);

/* TYPE_USER_STRUCT */
struct GTY((user)) user_struct {
  int data;
  void *extra;
};

void gt_ggc_mx_user_struct(void *p);
void gt_pch_nx_user_struct(void *p);
void gt_pch_p_16user_struct(void *p);

/* TYPE_STRUCT with various fields */
struct GTY(()) test_struct {
  /* Scalar fields */
  int id;
  unsigned long flags;
  
  /* Pointer fields with GC roots */
  struct test_struct *GTY((skip)) next_skipped;
  struct test_struct *GTY((chain_next("%s.next"))) next;
  struct test_struct *GTY((chain_prev("%s.prev"))) prev;
  
  /* String field */
  const char *GTY((length("%s.name_len"))) name;
  size_t name_len;
  
  /* Array field */
  int *GTY((length("%s.array_len"))) dynamic_array;
  size_t array_len;
  
  /* Nested structure */
  struct inner_struct *GTY((tag("0"))) inner;
  
  /* Union field */
  union test_union *GTY((desc("%s.type"))) variant;
  
  /* Callback field */
  complex_callback callback;
  
  /* User struct field */
  struct user_struct *GTY((skip)) user_data;
  
  /* For discriminated union */
  enum struct_type type;
};

/* TYPE_UNION */
union GTY((desc("%d.type"))) test_union {
  struct GTY((tag("TYPE_INT"))) {
    int int_value;
  } as_int;
  
  struct GTY((tag("TYPE_PTR"))) {
    struct test_struct *GTY((skip)) ptr_value;
  } as_ptr;
  
  struct GTY((tag("TYPE_STRING"))) {
    const char *GTY((length("%s.str_len"))) string_value;
    size_t str_len;
  } as_string;
  
  int type;
};

/* TYPE_ARRAY - Various array types */
struct GTY(()) array_container {
  /* Fixed-length array */
  int GTY(()) fixed_array[10];
  
  /* Variable-length array */
  struct test_struct **GTY((length("%s.var_len"))) var_array;
  size_t var_len;
  
  /* Nested array */
  int **GTY((length("%s.nested_len"))) nested_array;
  size_t nested_len;
};

/* TYPE_POINTER - Various pointer types */
typedef struct test_struct *GTY(()) test_struct_ptr;
typedef union test_union *GTY(()) test_union_ptr;
typedef int *GTY(()) int_ptr;
typedef const char **GTY(()) string_array_ptr;

/* Linked list structure for chain_next/chain_prev testing */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((chain_next("%s.next"))) next;
  struct linked_list *GTY((chain_prev("%s.prev"))) prev;
};

/* Param_is example */
struct GTY((param_is(T))) template_like {
  T *GTY((skip)) data;
  size_t count;
};

/* TYPE_LANG_STRUCT - Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) c_tree_node {
  enum tree_code code : 16;
  union tree_node *GTY((tag("0"))) chain;
  tree_node *GTY((skip)) next;
};
#endif

/* Nested complex type */
struct GTY(()) outer_container {
  struct test_struct main;
  union test_union variants[5];
  struct array_container arrays;
  struct linked_list *GTY((skip)) list_head;
};

/* Incomplete type usage to trigger TYPE_UNDEFINED */
struct GTY(()) uses_forward {
  forward_ptr GTY((maybe_undef)) forward_ref;
  struct forward_declared_struct *GTY((maybe_undef)) another_forward;
};

/* Complete the forward declaration */
struct GTY(()) forward_declared_struct {
  int completed;
  struct uses_forward *back_ref;
};

#endif /* TEST_GTYPE_H */
