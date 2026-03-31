/* Test file to cover all gengtype-state.cc type serialization cases.
   This file should be included in GCC's build to ensure all TYPE_*
   enum values are processed by gengtype. */

#ifndef GCC_TEST_COVERAGE_TYPES_H
#define GCC_TEST_COVERAGE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Multiple struct types with nested fields */
struct GTY(()) base_struct {
  int GTY((skip)) scalar_field;  /* TYPE_SCALAR */
  char * GTY((length("strlen(%h.ptr_field)+1"))) ptr_field;  /* TYPE_STRING */
  struct base_struct * GTY((tag("0"))) next;  /* TYPE_POINTER */
};

/* Another struct with union field */
struct GTY(()) complex_struct {
  int id;
  
  /* TYPE_UNION inside struct */
  union GTY((desc("%1.type"))) data_union {
    int GTY((tag("0"))) int_val;
    float GTY((tag("1"))) float_val;
    char * GTY((tag("2"))) string_val;
    struct base_struct * GTY((tag("3"))) struct_ptr;
  } data;
  
  /* TYPE_ARRAY field */
  int GTY(()) int_array[10];
  
  /* Incomplete array */
  struct base_struct * GTY(()) ptr_array[];
};

/* TYPE_UNION: Standalone union type */
union GTY(()) standalone_union {
  int i;
  double d;
  void * GTY((skip)) p;
  struct complex_struct * GTY((chain_next("%h.next"))) s;
};

/* TYPE_POINTER: Various pointer types */
typedef int * GTY(()) int_ptr;
typedef void (* GTY(()) void_func_ptr)(void);
typedef struct base_struct * GTY(()) base_struct_ptr;

/* TYPE_ARRAY: Different array types */
extern int GTY(()) external_array[];
static char GTY(()) static_char_array[] = "Hello World";  /* TYPE_STRING */
const float GTY(()) const_float_array[5] = {1.0, 2.0, 3.0, 4.0, 5.0};

/* TYPE_SCALAR: Various scalar types */
typedef enum GTY(()) color_enum {
  RED,
  GREEN,
  BLUE
} color;

typedef _Bool GTY(()) boolean;
typedef long long GTY(()) long_long_type;

/* TYPE_STRING: String types with literals */
const char GTY(()) *global_string = "Global string constant";
static const char GTY(()) static_string[] = "Static string array";

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int (* GTY((callback)) compare_func)(const void *, const void *);
typedef void (* GTY((callback)) traverse_func)(struct base_struct *, void *);

/* TYPE_LANG_STRUCT: GCC internal/lang-specific structures */

/* Vector type using GCC extension */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node * GTY((skip)) chain;
  union tree_node * GTY((skip)) type;
};

struct GTY(()) tree_decl_minimal {
  struct tree_common common;
  location_t locus;
  unsigned int uid;
};

/* TYPE_USER_STRUCT: User-defined structure type with special handling */
struct GTY((user)) user_defined_struct {
  /* The "user" tag marks this for special processing */
  int magic_number;
  void * GTY((skip)) user_data;
  
  /* Callback field */
  traverse_func GTY((callback)) callback;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
  /* Array of pointers to structs */
  struct base_struct * GTY(()) struct_ptrs[5];
  
  /* Pointer to array */
  int (* GTY(())) array_ptr)[10];
  
  /* Function pointer returning struct pointer */
  struct complex_struct * (* GTY((callback)) alloc_func)(size_t);
  
  /* Union containing various types */
  union {
    int GTY((tag("0"))) as_int;
    struct user_defined_struct * GTY((tag("1"))) as_user;
    compare_func GTY((tag("2"))) as_callback;
  } variant;
  
  /* Nested struct */
  struct {
    int depth;
    struct container_struct * GTY((skip)) parent;
  } nesting;
};

/* Global variables to ensure types are referenced */
extern struct base_struct GTY(()) *global_base_ptr;
extern union standalone_union GTY(()) global_union;
extern int_ptr GTY(()) global_int_ptr_array[3];
extern compare_func GTY(()) global_comparator;

/* Inline function using the types (not for gengtype, but to ensure usage) */
static inline void GTY((callback)) process_struct(struct base_struct *s) {
  if (s && s->ptr_field)
    printf("%s\n", s->ptr_field);
}

#endif /* GCC_TEST_COVERAGE_TYPES_H */
