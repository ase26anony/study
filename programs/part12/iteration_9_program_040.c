/* test-gtype-coverage.h - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during build */

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
typedef long long GTY(()) scalar_ll;

/* TYPE_STRING: String types */
typedef const char * GTY(()) string_ptr;
extern const char GTY(()) test_string[] = "Hello, gengtype!";
static const char * GTY(()) static_string = "Static string";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *);
typedef void * GTY((callback)) (*alloc_fn)(size_t);

/* TYPE_POINTER: Various pointer types */
typedef scalar_int * GTY(()) int_ptr;
typedef void * GTY(()) void_ptr;
typedef struct test_struct * GTY(()) struct_ptr;
typedef union test_union * GTY(()) union_ptr;
typedef compare_fn * GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
extern int GTY(()) extern_array[];
typedef struct test_struct GTY(()) struct_array[5];
typedef void * GTY(()) ptr_array[];

/* TYPE_UNION: Union types */
union GTY(()) test_union {
  int i;
  float f;
  double d;
  void *p;
  struct test_struct *s;
};

union GTY(()) nested_union {
  union test_union u;
  int_array a;
  compare_fn fn;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) test_struct {
  /* Scalar fields */
  scalar_int id;
  color_enum color;
  
  /* Pointer fields */
  int_ptr numbers;
  struct test_struct * GTY((skip)) next;
  struct test_struct * GTY((chain_next("%h.next"))) chain_next;
  
  /* Array field */
  int_array values;
  
  /* Union field */
  union test_union data;
  
  /* String field */
  string_ptr name;
  
  /* Callback field */
  compare_fn comparator;
};

/* More complex struct with nested types */
struct GTY(()) complex_struct {
  /* Nested struct */
  struct GTY(()) inner_struct {
    int x;
    int y;
    struct inner_struct * GTY((skip)) parent;
  } inner;
  
  /* Array of pointers */
  struct test_struct * GTY(()) items[8];
  
  /* Flexible array member */
  int GTY((length("%h.count"))) flexible[];
  int count;
  
  /* Union with struct */
  union {
    struct test_struct s;
    union test_union u;
  } GTY(()) variant;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* Using GCC-specific extensions and attributes */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Aligned type with attribute */
struct GTY(()) aligned_struct {
  int data;
} __attribute__((aligned(32)));

/* GCC internal tree-like structure (simplified) */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node * GTY((skip)) chain;
  union tree_node * GTY((skip)) type;
};

struct GTY(()) tree_decl {
  struct tree_common common;
  const char * GTY((skip)) name;
  struct tree_decl * GTY((chain_next("%h.next"))) next;
};

/* Union of tree types (common in GCC) */
union GTY(()) tree_node {
  struct tree_common common;
  struct tree_decl decl;
  /* Add more tree types here if needed */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* This would typically be in a language frontend */
struct GTY(()) lang_type {
  struct tree_common base;
  void * GTY((skip)) language_specific;
  unsigned int lang_flag1 : 1;
  unsigned int lang_flag2 : 1;
};

/* Recursive type structure to ensure deep traversal */
struct GTY(()) recursive_node {
  int value;
  struct recursive_node * GTY((skip)) left;
  struct recursive_node * GTY((skip)) right;
  struct recursive_node * GTY((chain_next("%h.next"))) next;
  
  /* Union with different pointer types */
  union {
    struct recursive_node *node;
    void *generic;
    string_ptr str;
  } GTY(()) link;
};

/* Array of unions */
union GTY(()) variant_array[4];

/* Struct containing array of callbacks */
struct GTY(()) callback_container {
  compare_fn GTY(()) comparators[3];
  traverse_fn GTY(()) traverser;
  alloc_fn GTY(()) allocator;
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) level1_int;
typedef level1_int GTY(()) level2_int;
typedef level2_int GTY(()) final_int;

/* Incomplete array in struct */
struct GTY(()) with_incomplete_array {
  int count;
  int GTY((length("%h.count"))) items[];
};

/* Multiple indirection pointers */
typedef struct test_struct *** GTY(()) triple_ptr;

/* Function returning struct by value */
struct test_struct GTY((returns_struct)) make_test_struct(int id);

/* Variable declarations using our types */
extern struct test_struct GTY(()) global_struct;
extern union test_union GTY(()) global_union;
extern int_array GTY(()) global_array;
extern compare_fn GTY(()) global_callback;

#endif /* TEST_GTYPE_COVERAGE_H */
