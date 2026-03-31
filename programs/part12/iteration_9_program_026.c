/* test-gtype-coverage.h - Comprehensive type coverage for gengtype testing */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) simple_struct {
  int a;
  char b;
};

struct GTY((chain_next ("%h.next"))) linked_list {
  int value;
  struct linked_list * GTY((skip)) next;
};

struct GTY(()) complex_struct {
  /* TYPE_SCALAR fields */
  int int_field;
  char char_field;
  long long_field;
  unsigned int uint_field;
  
  /* TYPE_POINTER fields */
  void * GTY((skip)) void_ptr;
  struct simple_struct * GTY((skip)) struct_ptr;
  
  /* TYPE_ARRAY field */
  int fixed_array[10];
  
  /* TYPE_STRING field */
  const char * GTY((skip)) string_ptr;
  char string_array[32];
  
  /* Nested TYPE_UNION */
  union {
    int as_int;
    float as_float;
    void * GTY((skip)) as_ptr;
  } GTY((tag ("0"))) nested_union;
};

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  double d;
  void * GTY((skip)) p;
};

union GTY((desc ("%1.int_field"))) tagged_union {
  struct {
    int tag;
    union {
      int int_val;
      float float_val;
      char * GTY((skip)) string_val;
    } GTY((tag ("0"))) data;
  } GTY((tag ("0"))) with_tag;
  long long_rep;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY((skip)) int_ptr;
typedef void (* GTY((skip)) void_func_ptr)(void);
typedef struct complex_struct * GTY((skip)) complex_ptr;

/* TYPE_ARRAY: Various array types */
extern int GTY((skip)) external_array[];
static int GTY((skip)) static_array[20];
int GTY((skip)) global_array[5] = {1, 2, 3, 4, 5};

/* TYPE_SCALAR: Various scalar types and enums */
typedef enum { RED, GREEN, BLUE, ALPHA } color_enum;
typedef _Bool bool_type;
typedef long long long_long_type;

/* TYPE_STRING: String types with initialization */
const char * GTY((skip)) global_string = "Hello, gengtype!";
char GTY((skip)) initialized_string[] = "Test string";
static const char * GTY((skip)) static_string = "Static string";

/* TYPE_CALLBACK: Function pointer types */
typedef int (* GTY((callback)) compare_func)(const void *, const void *);
typedef void (* GTY((callback)) traverse_func)(void * GTY((skip)), void *);

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
/* Using GCC vector extension to potentially trigger TYPE_USER_STRUCT */
typedef int GTY((skip)) v4si __attribute__((vector_size(16)));

struct GTY(()) user_marked_struct {
  v4si vector_data;
  int regular_field;
};

/* TYPE_LANG_STRUCT: GCC internal language structures */
/* Mimic tree-like structure from GCC internals */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node * GTY((skip)) chain;
  union tree_node * GTY((skip)) type;
  int uid;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  HOST_WIDE_INT int_cst_low;
  unsigned HOST_WIDE_INT int_cst_high;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container {
  /* Array of pointers to structs */
  struct simple_struct * GTY((skip)) struct_array[5];
  
  /* Pointer to array */
  int (* GTY((skip)) array_ptr)[10];
  
  /* Function pointer returning struct */
  struct complex_struct * (* GTY((callback)) alloc_func)(size_t);
  
  /* Union containing struct */
  union {
    struct simple_struct s;
    struct complex_struct c;
    int i;
  } GTY((tag ("0"))) data;
  
  /* Pointer to incomplete type (TYPE_UNDEFINED) */
  struct opaque_struct * GTY((skip)) opaque_ptr;
  
  /* Callback in struct */
  compare_func comparator;
};

/* Recursive structure */
struct GTY(()) tree_node {
  int value;
  struct tree_node * GTY((skip)) left;
  struct tree_node * GTY((skip)) right;
  struct tree_node * GTY((skip)) parent;
};

/* Another TYPE_UNION example with nested arrays */
union GTY(()) data_container {
  struct {
    int count;
    char buffer[256];
  } GTY((tag ("0"))) as_struct;
  long as_long[32];
  void * GTY((skip)) as_pointers[16];
};

/* TYPE_ARRAY of unions */
union GTY(()) variant_array[10];

/* TYPE_POINTER to function returning pointer to array */
int (* GTY((callback)) (* GTY((skip)) complex_func_ptr)(void))[5];

/* Complete the TYPE_UNDEFINED forward declaration */
struct GTY(()) opaque_struct {
  int revealed;
  struct container * GTY((skip)) link;
};

/* Global variables with various types for gengtype to process */
extern struct container GTY((skip)) global_container;
extern union simple_union GTY((skip)) global_union;
extern color_enum GTY((skip)) global_enum;
extern compare_func GTY((skip)) global_callback;

/* Inline function using the types (not processed by gengtype but valid C) */
static inline void init_types(void) {
  static struct simple_struct local_struct = {1, 'A'};
  static union simple_union local_union = {.i = 42};
  (void)local_struct;
  (void)local_union;
}

#endif /* TEST_GTYPE_COVERAGE_H */
