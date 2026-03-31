/* Test header to cover gengtype-state.cc switch cases */
#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration for opaque type */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  void * GTY((skip)) field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  void* data;
  struct my_struct * GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  void* p;
  struct my_struct * GTY((tag("1"))) s;
};

/* TYPE_POINTER: Pointer typedef */
typedef struct my_struct * GTY(()) my_ptr;
typedef union my_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array */
typedef int GTY(()) int_array[10];
typedef struct my_struct * GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_code { TEST_NODE_A, TEST_NODE_B, TEST_NODE_C };

struct GTY((desc("TEST_NODE"))) lang_struct {
  enum test_node_code code;
  union GTY((desc ("%1.code"))) {
    struct GTY((tag("0"))) {
      int int_val;
    } int_node;
    struct GTY((tag("1"))) {
      const char * GTY((length("strlen(%0)+1"))) str_val;
    } str_node;
    struct GTY((tag("2"))) {
      struct lang_struct * GTY((skip("code"))) child;
    } parent_node;
  } u;
};

/* TYPE_SCALAR: Fundamental scalar type */
extern GTY(()) int global_scalar;
extern GTY(()) unsigned long global_ulong;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;
extern GTY(()) char* mutable_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
typedef int (* GTY(()) int_callback)(int, const char*);

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains pointer */
  my_ptr first_ptr;
  
  /* Contains union */
  union my_union data_union;
  
  /* Contains array */
  int_array number_array;
  
  /* Contains pointer to array */
  struct_ptr_array * GTY((skip)) ptr_to_array;
  
  /* Contains language struct */
  struct lang_struct lang_data;
  
  /* Chain pointers */
  struct complex_nested * GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
  struct complex_nested * GTY((skip)) prev;
  
  /* Callback */
  callback_fn handler;
  
  /* String */
  const char * GTY((length("strlen(%0)+1"))) name;
  
  /* Reference to undefined type */
  struct opaque_struct * GTY((skip)) opaque_ref;
};

/* TYPE_UNDEFINED: Now define the previously opaque struct */
struct GTY(()) opaque_struct {
  int defined_field;
  struct complex_nested * GTY((skip)) complex_ref;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested *global_complex_ptr;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) callback_fn global_callback;
extern GTY(()) int_array global_int_array;

/* Struct with length option for array */
struct GTY(()) variable_length_struct {
  int count;
  int GTY((length("%0.count"))) values[1];
};

/* Struct with skip option */
struct GTY(()) skip_example {
  int important;
  void * GTY((skip)) skip_this;
  struct skip_example * GTY((skip("important"))) next_skip;
};

/* Union with tag */
union GTY((tag("TYPE"))) tagged_union {
  int type;
  struct GTY((tag("1"))) {
    int x;
    int y;
  } point;
  struct GTY((tag("2"))) {
    const char *name;
    int value;
  } named;
};

#endif /* TEST_COVERAGE_H */
