/* test-gtype-coverage.c - Comprehensive type declarations for gengtype coverage
   This file should be placed in the gcc/ directory and processed during GCC build.
   It contains diverse GTY-annotated types to trigger all TYPE_* cases in write_state_type.
*/

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef long GTY(()) scalar_long;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_literal = "test string";
char GTY(()) string_array[] = "array string";

/* TYPE_POINTER: Various pointer types */
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct opaque_struct* GTY(()) opaque_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void*, const void*);
typedef void GTY((callback)) (*void_callback)(void);

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef int GTY(()) array_of_ints[5];
typedef char GTY(()) string_array_type[32];

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
  int GTY(()) field1;
  char GTY(()) field2;
  scalar_int GTY(()) field3;
};

struct GTY(()) nested_struct {
  struct simple_struct GTY(()) inner;
  int_ptr GTY(()) ptr_field;
  array_of_ints GTY(()) array_field;
};

/* Recursive struct with chain_next */
struct GTY((chain_next("%h.next"))) linked_node {
  int GTY(()) data;
  struct linked_node* GTY(()) next;
  compare_func GTY(()) comparator;
};

/* Struct with union field */
struct GTY(()) struct_with_union {
  int GTY(()) type;
  union {
    int GTY(()) int_val;
    char* GTY(()) str_val;
    void* GTY(()) ptr_val;
  } GTY(()) value;
};

/* TYPE_UNION: Union types */
union GTY(()) simple_union {
  int GTY(()) i;
  float GTY(()) f;
  char* GTY(()) s;
  void* GTY(()) p;
};

union GTY(()) complex_union {
  struct simple_struct GTY(()) s;
  union simple_union GTY(()) u;
  array_of_ints GTY(()) a;
  compare_func GTY(()) f;
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
struct GTY((user)) user_defined_struct {
  int GTY(()) tag;
  union {
    struct simple_struct GTY(()) s;
    union simple_union GTY(()) u;
  } GTY(()) data;
  void (*GTY((skip)) custom_serialize)(void*);
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific types */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  int GTY(()) code;
  union tree_node* GTY(()) chain;
  union tree_node* GTY(()) type;
};

struct GTY(()) tree_int_cst {
  struct tree_common GTY(()) common;
  long GTY(()) int_cst_low;
  long GTY(()) int_cst_high;
};

union GTY((desc("TREE_CODE(%0.code)"))) tree_node {
  struct tree_common GTY(()) common;
  struct tree_int_cst GTY(()) int_cst;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
  int GTY(()) code;
  union {
    long GTY(()) intval;
    char* GTY(()) str;
    struct rtx_def* GTY(()) rt_rtx;
  } GTY(()) u;
  struct rtx_def* GTY(()) next;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) master_container {
  /* All type kinds in one struct */
  struct simple_struct GTY(()) regular_struct;
  union simple_union GTY(()) regular_union;
  struct user_defined_struct GTY(()) user_struct;
  union tree_node GTY(()) lang_struct;
  int_ptr GTY(()) pointer_field;
  array_of_ints GTY(()) array_field;
  scalar_int GTY(()) scalar_field;
  const char* GTY(()) string_field;
  compare_func GTY(()) callback_field;
  v4si GTY(()) vector_field;
  
  /* Nested containers */
  struct master_container* GTY(()) next;
  struct master_container* GTY(()) prev;
};

/* Additional pointer types for coverage */
typedef struct master_container* GTY(()) container_ptr;
typedef union tree_node* GTY(()) tree_ptr;
typedef struct rtx_def* GTY(()) rtx_ptr;

/* Array of various pointers */
struct master_container* GTY(()) container_array[5];
union tree_node* GTY(()) tree_array[3];

/* Function returning struct */
struct simple_struct GTY(()) (*func_returning_struct)(void);

/* Pointer to array */
typedef int (*GTY(()) array_ptr)[10];

/* Multi-dimensional array */
int GTY(()) matrix[3][4];

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
  unsigned int GTY(()) flag1 : 1;
  unsigned int GTY(()) flag2 : 2;
  unsigned int GTY(()) value : 29;
};

/* Union with bitfields */
union GTY(()) bitfield_union {
  struct {
    unsigned int GTY(()) a : 8;
    unsigned int GTY(()) b : 8;
    unsigned int GTY(()) c : 16;
  } GTY(()) parts;
  unsigned int GTY(()) whole;
};

/* Typedef chain */
typedef int GTY(()) base_type;
typedef base_type GTY(()) level1;
typedef level1 GTY(()) level2;
typedef level2* GTY(()) level3;

/* Opaque pointer typedef */
typedef struct opaque_struct* GTY(()) opaque_handle;

/* Self-referential types */
struct GTY(()) self_ref {
  int GTY(()) data;
  struct self_ref* GTY(()) self;
  struct self_ref* GTY(()) (*get_self)(void);
};

/* Complex callback with struct parameter */
typedef void GTY((callback)) (*complex_callback)(
  struct simple_struct*,
  union simple_union*,
  int
);

/* Struct using the complex callback */
struct GTY(()) callback_user {
  complex_callback GTY(()) cb;
  void* GTY(()) user_data;
};

/* Ensure all types are referenced to avoid optimization */
static void GTY(()) use_all_types(void) {
  static struct master_container GTY(()) container;
  static union tree_node GTY(()) tree;
  static struct rtx_def GTY(()) rtx;
  static struct user_defined_struct GTY(()) user;
  
  /* Reference them to ensure they're in the object file */
  (void)container;
  (void)tree;
  (void)rtx;
  (void)user;
}
