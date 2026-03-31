/* GCC internal type definitions for gengtype coverage testing.
   This file contains diverse type declarations to trigger all
   TYPE_* cases in write_state_type() serialization logic.
   
   Include this file in gcc/config.gcc or add to gtype-desc sources. */

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
typedef unsigned int GTY(()) scalar_uint;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types with literals */
extern const char GTY(()) *string_ptr = "test string literal";
static char GTY(()) string_array[] = "initialized string array";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(void *);
typedef void *GTY((callback)) (*alloc_fn)(size_t);

/* TYPE_POINTER: Various pointer types */
typedef scalar_int *GTY(()) int_ptr;
typedef void *GTY(()) void_ptr;
typedef struct GTY(()) my_struct *GTY(()) struct_ptr;
typedef compare_fn GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) incomplete_array[];
extern int GTY(()) fixed_array[10];
typedef int GTY(()) int_array[5];
typedef struct GTY(()) my_struct *GTY(()) ptr_array[8];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
  int GTY(()) i;
  float GTY(()) f;
  void *GTY(()) p;
  struct GTY(()) my_struct *GTY(()) s;
};

union GTY(()) tagged_union {
  enum { TAG_INT, TAG_FLOAT, TAG_PTR } GTY(()) tag;
  struct {
    int GTY(()) int_val;
  } GTY(()) i;
  struct {
    float GTY(()) float_val;
  } GTY(()) f;
  struct {
    void *GTY(()) ptr_val;
  } GTY(()) p;
};

/* TYPE_STRUCT: Regular struct types with nested fields */
struct GTY(()) my_struct {
  int GTY(()) id;
  char *GTY(()) name;
  union GTY(()) my_union GTY(()) data;
  struct GTY(()) my_struct *GTY(()) next;
  int GTY(()) values[5];
  compare_fn GTY(()) comparator;
};

struct GTY(()) nested_struct {
  struct GTY(()) {
    int GTY(()) x;
    int GTY(()) y;
  } GTY(()) point;
  
  union GTY(()) {
    int GTY(()) arr[3];
    struct GTY(()) {
      int GTY(()) a;
      int GTY(()) b;
    } GTY(()) pair;
  } GTY(()) data;
};

/* TYPE_USER_STRUCT: User-defined struct type with special handling */
struct GTY((user)) user_defined_struct {
  int GTY(()) magic;
  void *GTY(()) user_data;
  struct GTY(()) my_struct *GTY(()) gc_data;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */

/* Vector type using GCC extension (often mapped to lang_struct) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC's internal tree nodes */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) tree_node {
  enum tree_code {
    ERROR_MARK,
    IDENTIFIER_NODE,
    TREE_LIST,
    VOID_TYPE,
    INTEGER_TYPE
  } GTY(()) code;
  
  union GTY((desc("%0.code"))) tree_node_union {
    struct GTY((tag("0.ERROR_MARK"))) {
      const char *GTY(()) error_msg;
    } GTY(()) error;
    
    struct GTY((tag("1.IDENTIFIER_NODE"))) {
      const char *GTY(()) identifier;
      unsigned int GTY(()) length;
    } GTY(()) id;
    
    struct GTY((tag("2.TREE_LIST"))) {
      struct GTY(()) tree_node *GTY(()) purpose;
      struct GTY(()) tree_node *GTY(()) value;
      struct GTY(()) tree_node *GTY(()) chain;
    } GTY(()) list;
    
    struct GTY((tag("3.VOID_TYPE"))) {
      int GTY(()) align;
    } GTY(()) void_type;
    
    struct GTY((tag("4.INTEGER_TYPE"))) {
      int GTY(()) precision;
      int GTY(()) min;
      int GTY(()) max;
    } GTY(()) integer_type;
  } GTY(()) u;
  
  struct GTY(()) tree_node *GTY(()) next;
  struct GTY(()) tree_node *GTY(()) prev;
};

/* RTL-like structure */
struct GTY((desc("%0.code"))) rtx_def {
  enum rtx_code {
    UNKNOWN,
    REG,
    MEM,
    CONST_INT,
    CODE_LABEL
  } GTY(()) code;
  
  union GTY((tag("%0.code"))) {
    struct GTY((tag("1.REG"))) {
      unsigned int GTY(()) regno;
      enum machine_mode GTY(()) mode;
    } GTY(()) reg;
    
    struct GTY((tag("2.MEM"))) {
      struct GTY(()) rtx_def *GTY(()) addr;
      enum machine_mode GTY(()) mode;
    } GTY(()) mem;
    
    struct GTY((tag("3.CONST_INT"))) {
      HOST_WIDE_INT GTY(()) int_val;
    } GTY(()) const_int;
    
    struct GTY((tag("4.CODE_LABEL"))) {
      const char *GTY(()) label;
      int GTY(()) uid;
    } GTY(()) code_label;
  } GTY(()) u;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) complex_root {
  struct GTY(()) my_struct *GTY(()) struct_field;
  union GTY(()) my_union GTY(()) union_field;
  struct GTY(()) tree_node *GTY(()) tree_field;
  struct GTY(()) rtx_def *GTY(()) rtx_field;
  int GTY(()) scalar_field;
  const char *GTY(()) string_field;
  compare_fn GTY(()) callback_field;
  int GTY(()) array_field[7];
  struct GTY(()) complex_root *GTY(()) self_ptr;
  struct GTY(()) {
    int GTY(()) nested_a;
    struct GTY(()) {
      int GTY(()) deeply_nested;
    } GTY(()) inner;
  } GTY(()) anonymous_struct;
};

/* Template for generating multiple instances */
#define DECLARE_TEST_TYPE(name, id) \
  struct GTY(()) test_struct_##name { \
    int GTY(()) identifier; \
    struct GTY(()) complex_root *GTY(()) root; \
    union GTY(()) tagged_union GTY(()) tag; \
    v4si GTY(()) vector_data; \
  };

DECLARE_TEST_TYPE(a, 1)
DECLARE_TEST_TYPE(b, 2)
DECLARE_TEST_TYPE(c, 3)

/* External references to ensure TYPE_UNDEFINED is used */
extern struct GTY(()) opaque_struct *GTY(()) external_opaque;
extern union GTY(()) opaque_union *GTY(()) another_opaque;

/* Function declarations using callback types */
void GTY((callback)) traverse_structure(struct GTY(()) complex_root *root,
                                        traverse_fn callback);
struct GTY(()) my_struct *GTY(()) find_structure(int id,
                                                 compare_fn comparator);

#endif /* GCC_TEST_COVERAGE_TYPES_H */
