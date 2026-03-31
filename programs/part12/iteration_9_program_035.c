/* test-gtype-coverage.h - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and included in gtype-desc.c */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;  /* TYPE_SCALAR */
  char * GTY((length("strlen(%h.string_field) + 1"))) string_field; /* TYPE_STRING */
};

struct GTY((chain_next("%h.next"))) linked_struct {
  int value;
  struct linked_struct * GTY((skip)) next;
};

struct GTY(()) complex_struct {
  /* Nested struct */
  struct base_struct nested;
  
  /* Pointer field */
  void * GTY((tag("0"))) generic_pointer;
  
  /* Array field */
  int GTY((length("%h.array_len"))) dynamic_array[1];
  int array_len;
  
  /* Fixed array */
  double fixed_array[10];
};

/* TYPE_UNION: Union types */
union GTY(()) simple_union {
  int int_val;
  float float_val;
  char * GTY((length("strlen(%h.char_ptr) + 1"))) char_ptr;
};

union GTY((desc("%1.union_tag"))) tagged_union {
  int union_tag;
  struct {
    int type;
    union tagged_union * GTY((skip)) next;
  } GTY((tag("0"))) node;
  struct {
    int type;
    char * GTY((length("strlen(%h.data) + 1"))) data;
  } GTY((tag("1"))) data_node;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY((skip)) int_ptr;
typedef void (* GTY((callback)) void_func_ptr)(void);
typedef struct base_struct * GTY((skip)) struct_ptr;

/* TYPE_ARRAY: Array declarations */
extern int GTY(()) external_array[];
static int GTY(()) static_array[5] = {1, 2, 3, 4, 5};

struct GTY(()) array_container {
  /* Incomplete array member */
  int GTY((length("%h.count"))) flexible_array[];
  int count;
};

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum { RED, GREEN, BLUE, ALPHA } GTY(()) color_enum;
typedef _Bool bool_type;
typedef long long long_long_type;

/* TYPE_STRING: String type handling */
const char GTY(()) *const_string = "Hello, gengtype!";
char GTY(()) string_array[] = "Test string array";

/* TYPE_CALLBACK: Function pointer types */
typedef int (* GTY((callback)) compare_func)(const void *, const void *);
typedef void (* GTY((callback)) traverse_func)(void *data, void *user_data);

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
struct GTY((user)) user_defined_struct {
  int user_id;
  char *user_name;
  struct user_defined_struct * GTY((skip)) user_link;
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific structures */

/* Simulate tree-like structure (common in GCC) */
struct GTY((lang_struct)) tree_common_sim {
  int code;
  union {
    struct tree_common_sim * GTY((skip)) chain;
    char * GTY((length("strlen(%h.identifier) + 1"))) identifier;
  } GTY((desc("%1.code"))) u;
};

/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Simulate RTL structure */
struct GTY((lang_struct)) rtx_def_sim {
  int code;
  int mode;
  union {
    int int_val;
    struct rtx_def_sim * GTY((skip)) rt_ptr;
  } GTY((desc("%1.code"))) u;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) master_container {
  /* Direct struct */
  struct base_struct direct;
  
  /* Pointer to union */
  union simple_union * GTY((skip)) union_ptr;
  
  /* Array of struct pointers */
  struct complex_struct * GTY((length("%h.ptr_count"))) *ptr_array;
  int ptr_count;
  
  /* Callback function pointer */
  compare_func comparator;
  
  /* Nested array of arrays */
  int matrix[3][3];
  
  /* Reference to lang struct */
  struct tree_common_sim * GTY((skip)) tree_node;
  
  /* Vector type */
  v4si vector_data;
  
  /* String with special encoding */
  char * GTY((length("strlen(%h.utf8_string) + 1"))) utf8_string;
  
  /* Opaque pointer (TYPE_UNDEFINED when serialized) */
  struct opaque_struct * GTY((skip)) opaque;
  
  /* Self-reference for recursion */
  struct master_container * GTY((skip)) next;
};

/* Additional callback type variations */
typedef struct master_container * (* GTY((callback)) 
  allocator_func)(size_t size);
typedef void (* GTY((callback)) 
  destructor_func)(struct master_container *obj);

/* Union containing callback */
union GTY(()) callback_union {
  traverse_func traverser;
  compare_func comparer;
  allocator_func allocator;
};

/* Test structure with bitfields (scalar handling) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  signed int value : 8;
};

/* Array of unions */
union simple_union GTY(()) union_array[4];

/* Pointer to array */
typedef int (* GTY((skip)) array_ptr)[10];

/* Function returning struct by value (tests struct handling) */
struct base_struct GTY(()) make_base_struct(int val, const char *str);

/* Inline function definitions if needed in .c file */
#ifdef TEST_GTYPE_COVERAGE_C
struct base_struct GTY(()) make_base_struct(int val, const char *str) {
  struct base_struct bs;
  bs.scalar_field = val;
  bs.string_field = xstrdup(str);
  return bs;
}
#endif

#endif /* TEST_GTYPE_COVERAGE_H */
