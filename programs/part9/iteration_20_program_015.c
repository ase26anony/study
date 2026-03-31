/* test-gtype.h - Comprehensive test for gengtype type coverage */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* TYPE_STRUCT - Basic structure with multiple fields */
struct GTY(()) test_struct {
  int scalar_field;
  unsigned long another_scalar;
  struct test_struct *GTY((skip)) skip_ptr;  /* Skip from GC */
  struct test_struct *GTY((chain_next("%s.next"), chain_prev("%s.prev"))) next;
  struct test_struct *prev;
  const char *GTY((tag("0"))) string_field;
};

/* TYPE_USER_STRUCT - Structure with user-defined marking */
struct GTY((user)) user_struct {
  void *custom_data;
  int user_tag;
};

/* Forward declaration for TYPE_UNDEFINED test */
struct GTY((maybe_undef)) forward_declared;

/* TYPE_UNION - Discriminated union */
union GTY((desc("%d.tag"))) test_union {
  int tag;
  struct {
    int GTY((tag("1"))) int_value;
  } as_int;
  struct {
    const char *GTY((tag("2"))) string_value;
  } as_string;
  struct {
    struct test_struct *GTY((tag("3"))) struct_ptr;
  } as_struct;
};

/* TYPE_POINTER - Various pointer types */
typedef struct test_struct *GTY(()) test_struct_ptr;
typedef union test_union *GTY(()) test_union_ptr;
typedef int *GTY(()) int_ptr;

/* TYPE_ARRAY - Fixed and variable length arrays */
struct GTY(()) array_container {
  int GTY((length("%d.length"))) *variable_array;
  int length;
  struct test_struct GTY((length("5"))) fixed_array[5];
  struct test_struct *GTY((length("%d.ptr_count"))) *ptr_array;
  int ptr_count;
};

/* TYPE_LANG_STRUCT - Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) c_tree_node {
  enum tree_code code;
  union tree_node *GTY((tag("0"))) operands[3];
  location_t locus;
};
#endif

/* TYPE_SCALAR - Various scalar types */
typedef int GTY(()) test_int;
typedef unsigned long GTY(()) test_ulong;
typedef size_t GTY(()) test_size_t;
typedef ptrdiff_t GTY(()) test_ptrdiff_t;

/* TYPE_STRING - String type */
typedef const char *GTY(()) test_string;

/* TYPE_CALLBACK - Callback function pointer */
typedef void (*GTY((callback)) test_callback)(
  struct test_struct *GTY(()),
  int,
  const char *GTY(())
);

/* Complex nested structure for deep traversal */
struct GTY(()) complex_nested {
  struct array_container GTY(()) container;
  union test_union GTY(()) data;
  test_callback GTY(()) callback;
  struct complex_nested *GTY(()) next;
  struct user_struct GTY(()) user_data;
};

/* Linked list structure using chain_next/chain_prev */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((chain_next("%s.next"), chain_prev("%s.prev"))) next;
  struct linked_list *prev;
};

/* Template-like structure with param_is */
struct GTY(()) template_container {
  void *GTY((param_is(struct test_struct))) data;
  int count;
};

/* Another structure that uses the forward-declared type */
struct GTY(()) uses_forward {
  struct forward_declared *GTY(()) fwd_ptr;
  int ready;
};

/* Now define the forward-declared structure */
struct GTY(()) forward_declared {
  int defined_now;
  struct uses_forward *GTY(()) back_ref;
};

#endif /* TEST_GTYPE_H */
