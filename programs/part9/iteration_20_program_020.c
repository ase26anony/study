/* test-gtype.h - Test types for gengtype coverage */
#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#ifdef GENERATOR_FILE
#include "ansidecl.h"
#include "system.h"
#endif

/* Forward declarations to potentially create TYPE_UNDEFINED */
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
  int counter;
};

/* TYPE_STRUCT with various fields */
struct GTY(()) test_struct {
  /* Scalar fields */
  int id;
  unsigned long flags;
  
  /* Pointer fields with GC roots */
  struct test_struct * GTY((skip)) next;  /* skip from GC */
  struct test_struct * GTY((chain_next("%s.next"))) chain_next;
  struct test_struct * GTY((chain_prev("%s.chain_next"))) chain_prev;
  
  /* String field */
  const char * GTY((tag("0"))) name;
  
  /* Pointer to union */
  union test_union * GTY((tag("1"))) union_ptr;
  
  /* Array field */
  int * GTY((length("%s.array_len"))) dynamic_array;
  int array_len;
  
  /* Callback field */
  my_callback_t GTY((skip)) callback;
  
  /* For discriminated union */
  int GTY((desc("%s.type"))) discriminator;
  int type;
};

/* TYPE_UNION */
union GTY((desc("%d"))) test_union {
  int as_int;
  double as_double;
  struct test_struct * GTY((tag("0"))) as_struct;
  const char * GTY((tag("1"))) as_string;
  
  /* Nested union */
  union {
    int nested_int;
    float nested_float;
  } GTY((tag("2"))) nested;
};

/* TYPE_ARRAY - Variable length */
struct GTY(()) array_container {
  int count;
  struct test_struct * GTY((length("%s.count"))) items;
  
  /* Fixed length array */
  int GTY((skip)) fixed_array[10];
};

/* TYPE_POINTER - Various pointer types */
typedef struct test_struct *test_struct_ptr;
typedef union test_union *test_union_ptr;
typedef my_callback_t *callback_ptr;

/* Linked list structure using chain_next/chain_prev */
struct GTY(()) linked_list {
  int value;
  struct linked_list * GTY((chain_next("%s.next"))) next;
  struct linked_list * GTY((chain_prev("%s.prev"))) prev;
};

/* Structure with param_is annotation */
struct GTY((param_is(T))) param_struct {
  void * GTY((skip)) data;
  int type;
};

/* TYPE_LANG_STRUCT - Mimic GCC's tree structure */
#ifdef GENERATOR_FILE
struct GTY(()) c_tree_node {
  int code;
  union tree_node * GTY((tag("0"))) operands[2];
  const char * GTY((tag("1"))) filename;
  int lineno;
};
#endif

/* Another structure to create complex graph */
struct GTY(()) complex_node {
  int id;
  struct complex_node ** GTY((length("%s.child_count"))) children;
  int child_count;
  struct complex_node *parent;
  
  /* Union field */
  union {
    int int_val;
    double double_val;
    struct test_struct *struct_ptr;
  } GTY((desc("%s.value_type"))) value;
  int value_type;
};

/* Structure containing array of user structs */
struct GTY(()) user_struct_container {
  int count;
  struct user_defined_struct * GTY((length("%s.count"))) user_items;
};

#endif /* TEST_GTYPE_H */
