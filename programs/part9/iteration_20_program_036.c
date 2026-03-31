/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "ansidecl.h"
#include "system.h"

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

/* TYPE_USER_STRUCT with user-defined marking */
struct GTY((user)) user_defined_struct {
  void *data;
  size_t size;
};

/* TYPE_STRUCT with various fields */
struct GTY(()) test_struct {
  /* Scalar fields */
  int id;
  unsigned long flags;
  
  /* Pointer fields with GC roots */
  struct test_struct * GTY((skip)) next;  /* Skip from GC */
  struct test_struct * GTY((chain_next("%s.next"))) chain_next;
  struct test_struct * GTY((chain_prev("%s.chain_prev"))) chain_prev;
  
  /* String field */
  const char * GTY((tag("0"))) name;
  
  /* Array field */
  int * GTY((length("%s.array_len"))) dynamic_array;
  int array_len;
  
  /* Union field */
  union test_union * GTY((desc("%s.type"))) variant;
  int type;
  
  /* Callback field */
  my_callback_t callback;
  
  /* Nested structure */
  struct inner_struct *inner;
};

/* TYPE_UNION */
union GTY((desc("%d.type"))) test_union {
  int type;
  struct {
    int x;
    int y;
  } GTY((tag("1"))) point;
  struct {
    const char *str;
    int len;
  } GTY((tag("2"))) text;
  double GTY((tag("3"))) value;
};

/* TYPE_ARRAY examples */
struct GTY(()) array_container {
  /* Fixed-length array */
  int fixed_array[10];
  
  /* Variable-length array */
  struct test_struct ** GTY((length("%s.count"))) varray;
  int count;
  
  /* Nested array */
  int * GTY((length("%s.rows * %s.cols"))) matrix;
  int rows;
  int cols;
};

/* TYPE_POINTER variations */
typedef struct test_struct *test_struct_ptr;
typedef union test_union *test_union_ptr;
typedef int *int_ptr;
typedef const char **string_ptr_ptr;

/* Linked list structure for chain_next/prev testing */
struct GTY(()) linked_list {
  int value;
  struct linked_list * GTY((chain_next("%s.next"))) next;
  struct linked_list * GTY((chain_prev("%s.prev"))) prev;
};

/* Parameterized structure */
struct GTY((param_is(T))) param_struct {
  void * GTY((skip)) data;
  int length;
};

/* Discriminated union with desc */
struct GTY(()) tagged_union_container {
  int tag;
  union {
    int as_int;
    double as_double;
    const char *as_string;
  } GTY((desc("%d.tag"))) data;
};

/* TYPE_LANG_STRUCT - Mimic front-end structure */
#ifdef TEST_C_FRONTEND
struct GTY(()) c_tree_node {
  enum tree_code code;
  union c_tree_union *u;
  struct c_tree_node * GTY((chain_next("%s.chain"))) chain;
};

union GTY((desc("%d.code"))) c_tree_union {
  int dummy;
  struct {
    const char *name;
    int value;
  } identifier;
};
#endif

/* Forward declared now defined - was TYPE_UNDEFINED */
struct forward_declared_struct {
  int id;
  struct test_struct *ref;
};

#endif /* TEST_GTYPE_H */
