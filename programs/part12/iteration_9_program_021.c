/* test-gtype-coverage.h - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and included in gtype-desc sources */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_literal = "test_string";
char GTY(()) string_array[] = "array_string";

/* TYPE_POINTER: Various pointer types */
typedef void GTY(()) *void_ptr;
typedef int GTY(()) *int_ptr;
typedef struct GTY(()) my_struct *struct_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *);

/* TYPE_ARRAY: Array types */
extern int GTY(()) incomplete_array[];
int GTY(()) fixed_array[10] = {0};
typedef int GTY(()) int_array[5];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  float f;
  void *p;
  struct GTY(()) my_struct *s;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  int GTY(()) a;
  char GTY(()) *b;
  struct GTY(()) my_struct *next;
  union GTY(()) my_union u;
  int GTY(()) arr[3];
};

/* Chainable struct with GTY options */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
  int GTY(()) value;
  struct GTY(()) linked_node *next;
  struct GTY(()) linked_node *prev;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
  int GTY(()) data;
  void GTY((skip)) *skip_ptr;  /* Skip this field in GC */
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
};

union GTY(()) tree_node {
  struct tree_common common;
  /* Add other tree variants as needed */
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container {
  /* TYPE_STRUCT */
  struct GTY(()) my_struct embedded_struct;
  
  /* TYPE_UNION */
  union GTY(()) my_union embedded_union;
  
  /* TYPE_POINTER */
  struct GTY(()) container *self_ptr;
  
  /* TYPE_ARRAY */
  struct GTY(()) my_struct *GTY(()) ptr_array[5];
  
  /* TYPE_CALLBACK */
  compare_fn GTY(()) comparator;
  
  /* TYPE_SCALAR */
  color_enum GTY(()) color;
  
  /* TYPE_STRING */
  const char GTY(()) *name;
  
  /* Nested array of pointers to callbacks */
  traverse_fn GTY(()) handlers[3];
};

/* Recursive type structure */
struct GTY(()) tree_node_2 {
  int GTY(()) value;
  struct GTY(()) tree_node_2 *GTY((length("%h.child_count"))) *children;
  int GTY(()) child_count;
};

/* Union containing various types */
union GTY(()) type_union {
  struct GTY(()) my_struct s;
  union GTY(()) my_union u;
  void *p;
  int i;
  float f;
  compare_fn fn;
};

/* Array of unions */
union GTY(()) my_union GTY(()) union_array[4];

/* Pointer to array */
typedef int GTY(()) (*array_ptr)[10];

/* Function returning struct */
struct GTY(()) my_struct GTY((returns_struct)) make_struct(int x);

/* Opaque pointer type */
typedef struct GTY(()) opaque_struct *opaque_ptr;

#endif /* TEST_GTYPE_COVERAGE_H */
