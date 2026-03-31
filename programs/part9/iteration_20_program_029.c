/* test-gtype.h - Comprehensive test of gengtype type categories */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct;
typedef struct forward_declared_struct *forward_ptr;

/* TYPE_SCALAR examples */
typedef int GTY(()) scalar_int;
typedef unsigned long GTY(()) scalar_ulong;

/* TYPE_STRING */
typedef const char *GTY(()) string_type;

/* TYPE_CALLBACK */
typedef void (*callback_func)(struct test_struct *);
typedef callback_func GTY((callback)) callback_type;

/* TYPE_STRUCT with various fields */
struct GTY(()) test_struct {
  scalar_int id;
  string_type name;
  struct test_struct *GTY((skip)) next_skipped;
  struct test_struct *GTY((chain_next("%s.next"))) next;
  struct test_struct *GTY((chain_prev("%s.prev"))) prev;
  forward_ptr GTY((maybe_undef)) forward_ref;  /* May create TYPE_UNDEFINED */
  callback_type callback;
};

/* TYPE_USER_STRUCT with custom marking */
struct GTY((user)) user_struct {
  int data;
  void *GTY((skip)) private_data;
};

/* TYPE_UNION with discriminator */
union GTY((desc("tag"))) test_union {
  int GTY((tag("0"))) as_int;
  string_type GTY((tag("1"))) as_string;
  struct test_struct *GTY((tag("2"))) as_struct;
  int tag;
};

/* TYPE_ARRAY - variable length */
struct GTY(()) varray_struct {
  int length;
  struct test_struct * GTY((length("%s.length"))) items[];
};

/* TYPE_ARRAY - fixed length */
struct GTY(()) fixed_array_struct {
  struct test_struct * GTY((length("10"))) items[10];
};

/* TYPE_POINTER variations */
typedef struct test_struct *GTY(()) struct_ptr;
typedef union test_union *GTY(()) union_ptr;
typedef struct varray_struct *GTY(()) varray_ptr;

/* Nested structure for complex relationships */
struct GTY(()) container_struct {
  struct_ptr first;
  union_ptr variant;
  varray_ptr variable_array;
  struct user_struct *GTY(()) user_structs;
  int GTY((length("%s.count"))) count;
  struct container_struct *GTY(()) children[5];
};

/* Linked list example */
struct GTY(()) linked_list {
  int value;
  struct linked_list *GTY((chain_next("%s.next"))) next;
};

/* TYPE_LANG_STRUCT - mimic GCC's tree structure */
#ifdef TEST_LANG_STRUCT
struct GTY(()) c_tree_node {
  enum tree_code code;
  union tree_node *GTY((tag("0"))) operands[2];
  struct c_tree_node *GTY(()) chain;
};
#endif

/* Template-like structure with param_is */
struct GTY((param_is(T))) template_struct {
  T *GTY(()) data;
  int size;
};

/* Now define the forward-declared structure to resolve TYPE_UNDEFINED */
struct GTY(()) forward_declared_struct {
  int resolved;
  struct test_struct *GTY(()) link;
};

#endif /* TEST_GTYPE_H */
