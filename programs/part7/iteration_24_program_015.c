/* Test header to cover all TYPE_* cases in gengtype-state.cc switch statement */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
  int field1;
  double field2;
};

/* TYPE_USER_STRUCT: User-defined marking routines */
struct GTY((user)) user_struct {
  void* GTY((skip)) data;
  int GTY((skip)) length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int i;
  double d;
  void* GTY((tag("0"))) p;
};

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* GTY(()) my_ptr;
typedef union my_union* GTY(()) union_ptr;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
typedef struct my_struct* GTY(()) struct_ptr_array[5];

/* TYPE_LANG_STRUCT: Language-specific structure with tag */
struct GTY((desc("TEST_NODE"))) lang_struct {
  int code;
  union tree_node* GTY((tag("0"))) u;
  struct lang_struct* GTY((chain_next)) next;
};

/* TYPE_SCALAR: Fundamental scalar type as GC root */
extern GTY(()) int global_scalar;
extern GTY(()) long global_long;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;
extern GTY(()) char* mutable_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_fn)(void);
typedef int (*GTY(()) int_callback)(int, double);

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
  /* Contains various type kinds */
  struct my_struct GTY((tag("1"))) s;          /* TYPE_STRUCT */
  union my_union GTY((tag("2"))) u;            /* TYPE_UNION */
  my_ptr GTY((tag("3"))) ptr;                  /* TYPE_POINTER */
  int_array GTY((tag("4"))) arr;               /* TYPE_ARRAY */
  struct lang_struct* GTY((tag("5"))) lang;    /* TYPE_LANG_STRUCT */
  callback_fn GTY((tag("6"))) callback;        /* TYPE_CALLBACK */
  const char* GTY((tag("7"))) str;             /* TYPE_STRING */
  
  /* Chain for linked list */
  struct complex_nested* GTY((chain_next)) next;
  struct complex_nested* GTY((chain_prev)) prev;
};

/* TYPE_STRUCT with length option for variable-sized array */
struct GTY(()) var_struct {
  int count;
  struct my_struct GTY((length("%h.count"))) items[1];
};

/* TYPE_STRUCT with skip option */
struct GTY(()) skip_struct {
  int important;
  void* GTY((skip)) ignored;
  char* GTY((skip)) also_ignored;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct complex_nested* global_nested_list;
extern GTY(()) int_array global_int_array;
extern GTY(()) callback_fn global_callback;

/* Now define the previously opaque struct to resolve TYPE_UNDEFINED */
struct GTY(()) opaque_struct {
  int resolved;
  struct my_struct* GTY(()) data;
};

/* Array of pointers with chain_next for linked list simulation */
struct GTY(()) ptr_chain {
  struct my_struct* GTY((ptr_alias)) data;
  struct ptr_chain* GTY((chain_next)) next;
};

/* Union with nested struct */
union GTY(()) nested_union {
  struct {
    int type;
    void* GTY((tag("1"))) payload;
  } GTY((tag("0"))) s;
  long raw;
};

#endif /* TEST_COVERAGE_H */
