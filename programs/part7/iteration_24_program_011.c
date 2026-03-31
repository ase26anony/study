/* Test header to cover all TYPE_* cases in gengtype-state.cc */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Global scalar variable */
extern GTY(()) int global_scalar;

/* TYPE_STRING: Global string variable */
extern GTY(()) const char *global_string;

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (* GTY(()) callback_fn)(void);

/* TYPE_STRUCT: Plain struct */
struct GTY(()) my_struct {
  int field1;
  double field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void * GTY((skip)) data;
  size_t size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void *p;
  double d;
};

/* TYPE_POINTER: Typedef for pointer */
typedef struct my_struct * GTY(()) my_ptr;

/* TYPE_ARRAY: Fixed-size array */
typedef int GTY(()) int_array[10];

/* TYPE_LANG_STRUCT: Language-specific struct with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union GTY((tag("0"))) {
    int ival;
    struct lang_struct * GTY((tag("1"))) next;
  } u;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container {
  /* Nested struct */
  struct my_struct nested_struct GTY((skip));
  
  /* Pointer to union */
  union my_union * GTY(()) union_ptr;
  
  /* Array of pointers */
  struct my_struct * GTY(()) ptr_array[5];
  
  /* Chain of structures */
  struct container * GTY((chain_next("%h.next"))) next;
  struct container * GTY((chain_prev("%h.prev"))) prev;
  
  /* Callback member */
  callback_fn GTY((skip)) callback;
  
  /* Variable-length array with length */
  int count;
  int GTY((length("%h.count"))) flexible_array[];
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct container *global_container;
extern GTY(()) int_array global_int_array;
extern GTY(()) struct lang_struct *global_lang_struct;
extern GTY(()) callback_fn global_callback;

/* TYPE_UNDEFINED: Now define the opaque struct */
struct GTY(()) opaque_struct {
  int defined_field;
  struct opaque_struct * GTY((skip)) next;
};

#endif /* TEST_COVERAGE_H */
