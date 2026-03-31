/* Test coverage types for gengtype-state.cc
   This file defines types that trigger all TYPE_* cases in write_state_type()
   when processed by gengtype during GCC build. */

#ifndef GCC_TEST_COVERAGE_TYPES_H
#define GCC_TEST_COVERAGE_TYPES_H

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
extern const char GTY(()) *string_ptr;
static const char GTY(()) test_string[] = "test_string_literal";
typedef char GTY(()) string_array[32];

/* TYPE_POINTER: Various pointer types */
typedef void GTY(()) *void_ptr;
typedef int GTY(()) *int_ptr;
typedef struct GTY(()) my_struct *struct_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*void_callback)(void);
typedef struct GTY(()) my_struct* GTY((callback)) (*struct_factory)(int);

/* TYPE_ARRAY: Array types */
extern int GTY(()) incomplete_array[];
typedef int GTY(()) fixed_array[10];
typedef struct GTY(()) my_struct* GTY(()) struct_ptr_array[5];
typedef int GTY(()) multi_dim_array[3][4][5];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int GTY((tag("0"))) i;
  float GTY((tag("1"))) f;
  void GTY((tag("2"))) *p;
  struct GTY((tag("3"))) my_struct *s;
};

union GTY(()) tagged_union {
  enum { TAG_INT, TAG_FLOAT, TAG_STRING } GTY((skip)) tag;
  union {
    int GTY((tag("TAG_INT"))) i;
    float GTY((tag("TAG_FLOAT"))) f;
    const char GTY((tag("TAG_STRING"))) *str;
  } GTY((desc("%0.tag"))) u;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
  int GTY(()) id;
  char GTY(()) name[32];
  struct GTY(()) my_struct *GTY((chain_next("%h.next"))) next;
  union GTY(()) my_union data;
  compare_fn GTY(()) comparator;
  int GTY(()) values[8];
};

struct GTY(()) nested_struct {
  struct GTY(()) {
    int GTY(()) x;
    int GTY(()) y;
  } point;
  
  union GTY(()) {
    int GTY(()) i;
    double GTY(()) d;
  } value;
  
  struct GTY(()) my_struct *GTY(()) items[4];
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
typedef struct GTY(()) my_struct user_struct_t;
typedef union GTY(()) my_union user_union_t;

/* TYPE_LANG_STRUCT: GCC internal/lang-specific struct types */

/* Vector type (SIMD) - often treated as lang struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC's tree_node */
struct GTY(()) tree_common {
  enum tree_code GTY((skip)) code : 16;
  unsigned GTY((skip)) side_effects_flag : 1;
  unsigned GTY((skip)) constant_flag : 1;
  unsigned GTY((skip)) addressable_flag : 1;
  unsigned GTY((skip)) volatile_flag : 1;
  union GTY(()) tree_node *GTY((chain_next("%h.next"))) next;
};

union GTY(()) tree_node {
  struct GTY(()) tree_common common;
  struct GTY(()) tree_decl {
    struct GTY(()) tree_common common;
    const char GTY(()) *name;
    union GTY(()) tree_node *type;
  } decl;
  struct GTY(()) tree_type {
    struct GTY(()) tree_common common;
    union GTY(()) tree_node *name;
    unsigned GTY((skip)) precision : 16;
  } type;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
  int GTY((skip)) code : 16;
  int GTY((skip)) mode : 8;
  union GTY(()) {
    int GTY((tag("0"))) int_val;
    const char GTY((tag("1"))) *str;
    struct GTY((tag("2"))) rtx_def *rtx;
  } GTY((desc("GET_CODE(%0)"))) u;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) type_graph {
  int GTY(()) id;
  
  /* Self-referential pointer */
  struct GTY(()) type_graph *GTY((chain_next("%h.self"))) self;
  
  /* Pointer to different struct type */
  struct GTY(()) my_struct *GTY(()) data;
  
  /* Array of function pointers */
  compare_fn GTY(()) callbacks[3];
  
  /* Union containing various types */
  union GTY(()) {
    int GTY((tag("0"))) scalar;
    struct GTY((tag("1"))) my_struct *GTY(()) struct_ptr;
    int GTY((tag("2"))) array[5];
    void GTY((tag("3"))) (*func_ptr)(void);
  } GTY((desc("%0.id % 4"))) variant;
  
  /* Nested struct */
  struct GTY(()) {
    int GTY(()) depth;
    struct GTY(()) type_graph *GTY(()) parent;
  } GTY(()) nest;
};

/* Template-like structure for C++ frontend simulation */
#ifdef __cplusplus
struct GTY(()) template_struct {
  void GTY(()) *data;
  int GTY(()) size;
  
  /* Method pointers (C++ member function pointers) */
  void (GTY(()) *init)(struct template_struct *);
  void (GTY(()) *cleanup)(struct template_struct *);
};
#endif

/* Global variables to ensure types are instantiated */
extern struct GTY(()) my_struct global_struct;
extern union GTY(()) my_union global_union;
extern struct GTY(()) type_graph *GTY(()) global_graph;

/* Inline function using callback type */
static inline int GTY(()) test_callback(compare_fn cmp, void *a, void *b) {
  return cmp ? cmp(a, b) : 0;
}

#endif /* GCC_TEST_COVERAGE_TYPES_H */
