/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;  /* TYPE_SCALAR */
  char *string_field; /* TYPE_STRING via pointer */
};

struct GTY(()) nested_struct {
  struct base_struct *GTY((skip)) ptr_field; /* TYPE_POINTER */
  union variant *variant_ptr; /* Another pointer */
};

/* TYPE_USER_STRUCT: Struct with user-defined properties */
struct GTY((user)) user_struct {
  int id;
  /* Chain next pointer for garbage collection */
  struct user_struct *GTY((chain_next("%h.next"))) next;
};

/* TYPE_UNION: Union types */
union GTY(()) variant {
  int int_val;        /* TYPE_SCALAR */
  float float_val;    /* TYPE_SCALAR */
  char *GTY((tag("STRING"))) str_val; /* TYPE_STRING */
  struct base_struct *struct_ptr; /* TYPE_POINTER */
};

/* TYPE_POINTER: Various pointer types */
typedef int *GTY(()) int_ptr;
typedef void (*GTY(()) void_func_ptr)(void);
typedef struct base_struct *GTY(()) struct_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) external_array[];
int GTY(()) fixed_array[10] = {0};
struct base_struct *GTY(()) ptr_array[5];

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Using vector extension as an example of lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
  enum tree_code code;  /* TYPE_SCALAR (enum) */
  union tree_node *chain;
};

union GTY(()) tree_node {
  struct tree_common common;
  /* Other tree variants would go here */
};

/* TYPE_SCALAR: Various scalar types */
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;
typedef _Bool GTY(()) boolean_type;
typedef long long GTY(()) long_long_type;

/* TYPE_STRING: String types */
const char GTY(()) *const_string = "Hello, gengtype!";
char GTY(()) string_array[] = "Test string";

/* TYPE_CALLBACK: Function pointer types with callback attribute */
typedef int GTY((callback)) (*compare_fn)(const void *, const void *);
typedef void GTY((callback)) (*traverse_fn)(struct base_struct *);

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_type {
  /* Nested union */
  union {
    int int_data;
    struct {
      char *name;
      int value;
    } named_data;
  } data_union;
  
  /* Array of function pointers */
  compare_fn GTY((length("%h.count"))) *comparators;
  int count;
  
  /* Pointer to array of structs */
  struct base_struct *GTY((length("%h.arr_len"))) *struct_array;
  int arr_len;
  
  /* Callback function */
  traverse_fn traverser;
  
  /* Lang struct member */
  v4si vector_data;
  
  /* String field */
  const char *description;
};

/* Another struct with chain for GC testing */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_node {
  int id;
  char *data;
  struct linked_node *next;
  struct linked_node *prev;
  struct linked_node *GTY((skip)) skip_ptr; /* Skipped pointer */
};

/* Union with nested struct */
union GTY(()) complex_union {
  struct {
    int type;
    void *data;
  } tagged;
  v4si vectors[2];
  struct linked_node *node_list;
};

/* Global variable with initializer containing string */
struct GTY(()) global_struct {
  int version;
  const char *version_string;
} global_instance = {1, "gengtype-coverage-test"};

/* Function pointer table */
static compare_fn GTY(()) comparison_functions[] = {
  NULL
};

/* Incomplete array in struct */
struct GTY(()) flexible_struct {
  int length;
  char data[];  /* Flexible array member */
};

/* Typedef chain leading to scalar */
typedef int GTY(()) my_int;
typedef my_int GTY(()) my_int2;
typedef my_int2 GTY(()) final_int;

/* Struct with bitfields (scalar type) */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  unsigned int padding : 26;
};

/* Test case for param_is/param uses */
struct GTY(()) param_struct {
  int GTY((param_is(int))) *param_ptr;
  struct complex_type *GTY((param_is(struct complex_type))) complex_param;
};

/* Mark the end of types */
void* GTY(()) end_marker = NULL;
