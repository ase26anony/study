/* test-gtype-coverage.h - Comprehensive GTY type coverage for gengtype testing */
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
const char GTY(()) *string_ptr = "test string";
char GTY(()) string_array[] = "array string";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *);

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct *GTY(()) struct_ptr;
typedef void *GTY(()) void_ptr;
typedef int *GTY(()) int_ptr;
typedef compare_fn GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) extern_array[];
int GTY(()) fixed_array[10] = {0};
struct my_struct *GTY(()) ptr_array[5];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int i;
  float f;
  void *p;
  struct my_struct *s;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  /* Scalar fields */
  int id;
  color_enum color;
  
  /* Pointer fields */
  struct my_struct *GTY((skip)) next;
  void *data;
  
  /* Array field */
  int GTY((length("%h.count"))) *values;
  int count;
  
  /* Union field */
  union my_union variant;
  
  /* String field */
  const char *name;
  
  /* Callback field */
  traverse_fn callback;
};

/* Nested struct for complex type graph */
struct GTY(()) container {
  /* Array of structs */
  struct my_struct GTY((length("%h.num_items"))) items[10];
  int num_items;
  
  /* Pointer to union */
  union my_union *GTY((tag("union_type"))) union_ptr;
  
  /* 2D array */
  int GTY(()) matrix[3][3];
  
  /* Pointer to callback */
  compare_fn GTY(()) compar;
};

/* Recursive struct with chain_next */
struct GTY((chain_next("%h.next"))) linked_list {
  int value;
  struct linked_list *next;
  struct linked_list *prev;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* Using GCC vector extension to trigger lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int code;
  union tree_node *chain;
};

union GTY(()) tree_node {
  struct tree_common common;
  struct tree_decl decl;
};

struct GTY(()) tree_decl {
  struct tree_common common;
  const char *name;
  union tree_node *type;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
  int code;
  union rtunion_def *fld;
};

union GTY(()) rtunion_def {
  int rtint;
  struct rtx_def *rtx;
  const char *rtstr;
};

/* Complex type with nested arrays of pointers */
struct GTY(()) type_graph {
  /* Array of pointers to structs */
  struct my_struct *GTY((length("%h.ptr_count"))) *ptr_array;
  int ptr_count;
  
  /* Pointer to array of unions */
  union my_union (*union_matrix)[5];
  
  /* Callback array */
  compare_fn callbacks[3];
  
  /* Nested struct with string */
  struct {
    const char *label;
    int depth;
  } GTY(()) nested;
};

/* Function pointer returning struct */
struct my_struct *GTY(()) (*struct_factory)(int id, const char *name);

/* Union containing struct with callback */
union GTY(()) complex_union {
  struct {
    int type;
    traverse_fn handler;
    void *context;
  } GTY(()) handler_data;
  
  struct {
    int *data;
    size_t size;
  } GTY(()) buffer;
};

/* Incomplete array in struct */
struct GTY(()) flexible_array {
  int count;
  int data[];
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) derived_int;
typedef derived_int GTY(()) final_int;

/* Multiple indirection */
typedef struct my_struct ***GTY(()) triple_ptr;

/* Self-referential type */
struct GTY(()) self_ref {
  int value;
  struct self_ref *GTY((skip)) self;
};

/* Union with struct array */
union GTY(()) data_container {
  struct my_struct structs[2];
  int numbers[10];
  char *strings[5];
};

/* Global variables to ensure processing */
extern struct my_struct GTY(()) global_struct;
extern union my_union GTY(()) global_union;
extern int GTY(()) global_array[20];
extern compare_fn GTY(()) global_callback;

#endif /* TEST_GTYPE_COVERAGE_H */
