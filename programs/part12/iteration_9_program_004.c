/* Test coverage types for gengtype-state.cc serialization logic.
   This file defines types corresponding to all TYPE_* enum values
   to ensure complete coverage of write_state_type switch cases. */

#ifndef GCC_TEST_COVERAGE_TYPES_H
#define GCC_TEST_COVERAGE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque;
union unknown;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) simple_struct {
  int a;
  char b;
};

struct GTY(()) nested_struct {
  int id;
  struct simple_struct GTY((skip)) data;
  struct nested_struct *GTY((tag("0"))) next;
};

struct GTY((chain_next("%h.next_ptr"))) linked_struct {
  int value;
  struct linked_struct *next_ptr;
  void *GTY((skip)) user_data;
};

/* Struct with array field */
struct GTY(()) array_struct {
  int count;
  int GTY((length("%h.count"))) items[10];
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
typedef struct GTY((user)) user_defined_struct {
  int magic;
  void *GTY((skip)) private_data;
  const char *description;
} user_defined_struct_t;

/* TYPE_UNION: Various union types */
union GTY(()) simple_union {
  int i;
  float f;
  double d;
  void *p;
};

union GTY((desc("%1.type"), tag("0"))) tagged_union {
  int type;
  struct simple_struct GTY((tag("1"))) as_struct;
  union simple_union GTY((tag("2"))) as_union;
  int GTY((tag("3"))) as_array[5];
};

/* TYPE_POINTER: Various pointer types */
typedef int *GTY((skip)) int_ptr;
typedef const char *GTY((skip)) string_ptr;
typedef void (*GTY((skip)) void_func_ptr)(void);
typedef struct nested_struct *GTY((skip)) struct_ptr;

/* Pointer to incomplete type */
extern struct opaque *GTY((skip)) opaque_ptr;

/* TYPE_ARRAY: Various array types */
extern int GTY((skip)) external_array[];
static int GTY((skip)) static_array[20];
int GTY((skip)) global_array[15];

/* Array of pointers */
struct GTY(()) array_of_pointers {
  int count;
  void *GTY((length("%h.count"))) pointers[8];
};

/* Multi-dimensional array */
int GTY((skip)) matrix[3][4];

/* TYPE_SCALAR: Fundamental scalar types */
typedef char byte_t;
typedef short int short_t;
typedef long int long_t;
typedef long long int longlong_t;
typedef unsigned int uint_t;
typedef _Bool bool_t;
typedef float float_t;
typedef double double_t;

/* Enum type */
typedef enum GTY(()) color {
  RED,
  GREEN,
  BLUE,
  MAX_COLORS
} color_t;

/* TYPE_STRING: String types */
const char GTY((skip)) *const_message = "Hello, GCC!";
char GTY((skip)) mutable_string[] = "Mutable string";
static const char GTY((skip)) *static_string = "Static string";

/* Struct with string field */
struct GTY(()) string_struct {
  const char *GTY((skip)) name;
  char GTY((skip)) buffer[256];
};

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*iterate_func)(void *data, int index);
typedef struct simple_struct *GTY((callback)) (*allocator_func)(size_t size);

/* Callback in struct */
struct GTY(()) callback_container {
  compare_func GTY((skip)) comparator;
  iterate_func GTY((skip)) iterator;
  void *GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific types */

/* Vector type (GCC extension) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Simulated tree node structure (simplified) */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
  location_t locus;
};

/* Simulated RTL structure (simplified) */
struct GTY(()) rtx_def {
  int code;
  int mode;
  union rtunion_def {
    int GTY((tag("0"))) intval;
    const char *GTY((tag("1"))) str;
    struct rtx_def *GTY((tag("2"))) rtx;
  } GTY((desc("GET_CODE(%0)"))) u;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) complex_node {
  int id;
  
  /* Self-reference */
  struct complex_node *GTY((skip)) self;
  
  /* Array of pointers to same type */
  struct complex_node *GTY((length("%h.id % 5"))) children[5];
  
  /* Union field */
  union {
    int GTY((tag("0"))) as_int;
    double GTY((tag("1"))) as_double;
    struct string_struct GTY((tag("2"))) as_string_struct;
  } GTY((desc("%h.id & 3"))) data;
  
  /* Callback */
  compare_func GTY((skip)) sorter;
  
  /* Scalar array */
  int GTY((skip)) scores[10];
  
  /* String */
  const char *GTY((skip)) label;
};

/* TYPE_NONE should not be reachable in normal operation,
   but we include diverse types to ensure all other cases are covered */

/* Typedef chain leading to scalar */
typedef int base_int;
typedef base_int derived_int;
typedef derived_int final_int;

/* Template for generating multiple instances */
#define DECLARE_STRUCT_TYPE(name, field_type) \
  struct GTY(()) name##_struct { \
    int id; \
    field_type GTY((skip)) data; \
  }

DECLARE_STRUCT_TYPE(int_wrapper, int);
DECLARE_STRUCT_TYPE(ptr_wrapper, void*);
DECLARE_STRUCT_TYPE(array_wrapper, int[10]);

/* Global variables with various types */
extern struct simple_struct GTY((skip)) global_simple_struct;
extern union simple_union GTY((skip)) global_simple_union;
extern int_ptr GTY((skip)) global_int_ptr;
extern color_t GTY((skip)) global_color;

/* Function declarations using these types */
struct simple_struct GTY((skip)) *create_simple_struct(int a, char b);
void process_complex_node(struct complex_node *GTY((skip)) node);
compare_func GTY((skip)) get_default_comparator(void);

#endif /* GCC_TEST_COVERAGE_TYPES_H */
