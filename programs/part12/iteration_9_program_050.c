/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_undefined;
union opaque_union_undefined;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String types */
const char *string_ptr = "test string";
char string_array[] = "hello world";
const char * GTY(()) gc_string = "garbage collected string";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct my_struct *struct_ptr;
typedef int (*func_ptr_simple)(void);
typedef void (*func_ptr_complex)(int, char *);

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
int *pointer_array[5];
const char *string_array_ptr[] = {"one", "two", "three"};

/* TYPE_CALLBACK: Function pointer types with GTY callback annotation */
typedef int GTY((callback)) (*comparator_callback)(const void *, const void *);
typedef void GTY((callback)) (*traversal_callback)(void *data, void *user_data);

/* TYPE_STRUCT: Regular structures */
struct GTY(()) simple_struct {
  int a;
  char b;
  long c;
};

struct GTY((chain_next("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
  struct linked_struct * GTY((chain_next("%h.chain_next"))) chain_next;
};

struct GTY(()) nested_struct {
  struct simple_struct inner;
  struct linked_struct *list;
  int array[5];
  void *pointer;
};

/* TYPE_UNION: Union types */
union GTY(()) simple_union {
  int i;
  float f;
  double d;
  void *p;
};

union GTY(()) tagged_union {
  int type_tag;
  struct {
    int x;
    int y;
  } point;
  struct {
    char *name;
    int age;
  } person;
};

/* TYPE_USER_STRUCT: User-defined structures with special handling */
/* Using GCC vector extension to trigger special handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) user_defined {
  v4si vector_data;
  int regular_field;
};

/* TYPE_LANG_STRUCT: Language-specific structures */
/* Mimic GCC's tree structure for language-specific handling */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
  union tree_node *type;
  int uid;
};

struct GTY(()) tree_int_cst {
  struct tree_common common;
  HOST_WIDE_INT int_cst_low;
  unsigned HOST_WIDE_INT int_cst_high;
};

/* Complex type that combines multiple type kinds */
struct GTY(()) comprehensive_type {
  /* SCALAR */
  int id;
  enum { STATE_A, STATE_B, STATE_C } state;
  
  /* STRING */
  const char * GTY((skip)) name;
  
  /* POINTER */
  void *user_data;
  struct comprehensive_type * GTY((skip)) sibling;
  
  /* ARRAY */
  int scores[10];
  struct simple_struct * GTY((length("%h.count"))) items;
  unsigned count;
  
  /* UNION */
  union simple_union data;
  
  /* CALLBACK */
  traversal_callback traverse;
  
  /* Nested STRUCT */
  struct nested_struct nested;
  
  /* Reference to UNDEFINED */
  struct opaque_undefined *opaque_ref;
};

/* Array of pointers to comprehensive types */
struct comprehensive_type * GTY((length("%h.ptrs_count"))) ptr_array[];
unsigned ptrs_count;

/* Union containing various pointer types */
union GTY(()) pointer_union {
  int *int_ptr;
  char **string_ptr_ptr;
  struct comprehensive_type *comp_ptr;
  void (*func_ptr)(void);
};

/* Function pointer table (array of callbacks) */
traversal_callback GTY((length("%h.callback_count"))) callbacks[];
unsigned callback_count;

/* Recursive structure with multiple pointer types */
struct GTY(()) recursive_node {
  int value;
  struct recursive_node * GTY((skip)) left;
  struct recursive_node * GTY((skip)) right;
  struct recursive_node * GTY((chain_next("%h.next"))) next;
  void (* GTY((callback)) visit)(struct recursive_node *);
};

/* Another structure to ensure TYPE_USER_STRUCT coverage */
struct GTY(()) with_vector {
  v4si data;
  v4si mask;
  int flags;
};

/* Structure with array of unions */
struct GTY(()) union_array_container {
  union simple_union items[8];
  int count;
};

/* Opaque pointer type */
typedef struct comprehensive_type * GTY((user)) comp_ptr_user;

/* Test case for deep nesting */
struct GTY(()) level3 {
  int data;
};

struct GTY(()) level2 {
  struct level3 l3;
  int *pointer_array[3];
};

struct GTY(()) level1 {
  struct level2 l2;
  union tagged_union tu;
  comparator_callback cmp;
};

/* Global instances to ensure they're processed */
struct comprehensive_type GTY(()) global_comp;
struct recursive_node * GTY((length("5"))) global_nodes[5];
union pointer_union GTY(()) global_pointer_union;

/* Inline function using the types (not processed by gengtype but ensures compilation) */
static void
process_types(void)
{
  struct comprehensive_type local;
  local.id = 1;
  local.state = STATE_A;
  local.name = "test";
  
  if (local.traverse)
    local.traverse(&local, NULL);
}

/* Additional GCC-specific types that might trigger special cases */
#ifdef ENABLE_CHECKING
struct GTY(()) checking_type {
  int magic;
  void *data;
};
#endif

/* Forward declaration that creates TYPE_UNDEFINED */
struct forward_declared;

/* Structure that references forward declared type */
struct GTY(()) uses_forward {
  int valid;
  struct forward_declared *future;
};

/* Array type with callback elements */
typedef void (* GTY((callback)) action_callback)(int, void*);
action_callback GTY((length("%h.action_count"))) actions[];
unsigned action_count;

/* Final comprehensive structure using all type kinds */
struct GTY(()) all_types {
  /* SCALAR */
  int scalar_int;
  char scalar_char;
  _Bool scalar_bool;
  color_enum color;
  
  /* STRING */
  const char *string_field;
  char string_buffer[64];
  
  /* POINTER */
  void *void_ptr;
  int *int_ptr;
  struct all_types *self_ptr;
  func_ptr_complex complex_func_ptr;
  
  /* ARRAY */
  int int_array[20];
  struct simple_struct struct_array[5];
  void *pointer_array[10];
  
  /* STRUCT */
  struct nested_struct nested;
  struct linked_struct linked;
  
  /* UNION */
  union simple_union data_union;
  union pointer_union ptr_union;
  
  /* CALLBACK */
  comparator_callback compare;
  traversal_callback traverse;
  
  /* USER_STRUCT */
  v4si vector_data;
  
  /* Reference to undefined/forward declared */
  struct forward_declared *opaque;
  
  /* Array length marker */
  unsigned array_len;
  
  /* For variable length array */
  int GTY((length("%h.array_len"))) var_len_array[];
};
