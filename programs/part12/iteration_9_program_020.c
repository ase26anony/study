/* test-gtype-coverage.h - Comprehensive type definitions for gengtype coverage */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_undefined_type;

/* TYPE_STRUCT: Various struct types with GTY annotations */
struct GTY(()) base_struct {
  int scalar_field;
  char *string_field;
};

struct GTY(()) nested_struct {
  struct base_struct *GTY((skip)) base_ptr;
  union my_union *union_ptr;
  int array_field[5];
};

/* Complex struct with chain_next for GC */
struct GTY((chain_next ("%h.next"))) linked_struct {
  int id;
  char *GTY((tag ("0"))) name;
  struct linked_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined behavior */
typedef struct GTY((user)) user_struct {
  int custom_data;
  void (*GTY((skip)) custom_cleanup)(void*);
} user_struct_t;

/* TYPE_UNION: Various union types */
union GTY(()) my_union {
  int int_val;
  float float_val;
  double double_val;
  void *ptr_val;
  struct base_struct *struct_ptr;
};

/* Tagged union for GC */
union GTY((desc ("%1.type"), tag ("type"))) tagged_union {
  int type;
  struct {
    int type;
    int int_data;
  } GTY((tag ("1"))) int_case;
  struct {
    int type;
    char *string_data;
  } GTY((tag ("2"))) string_case;
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *generic_ptr;
typedef struct base_struct *struct_ptr;
typedef union my_union *union_ptr;

/* Function pointer types */
typedef int (*int_func_ptr)(int, int);
typedef void (*void_func_ptr)(void);
typedef struct base_struct *(*struct_creator_ptr)(void);

/* TYPE_ARRAY: Various array types */
extern int external_array[];
extern struct base_struct *struct_ptr_array[10];

/* Fixed-size arrays */
int fixed_int_array[20] = {0};
char fixed_char_array[50] = "This is a string literal";

/* Multi-dimensional array */
int matrix[3][3];

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int;
typedef long my_long;
typedef unsigned long my_ulong;
typedef _Bool my_bool;
typedef float my_float;
typedef double my_double;

/* Enum types */
typedef enum color {
  RED,
  GREEN,
  BLUE
} color_t;

typedef enum GTY(()) gty_enum {
  ENUM_VAL1,
  ENUM_VAL2,
  ENUM_VAL3
} gty_enum_t;

/* TYPE_STRING: String types */
const char *const_string = "Constant string literal";
char mutable_string[] = "Mutable string literal";
char *string_pointer = "Another string";

/* TYPE_CALLBACK: Callback/function pointer types */
typedef int GTY((callback)) (*compare_func)(const void *, const void *);
typedef void GTY((callback)) (*traversal_func)(void *data, void *user_data);

/* Callback with complex signature */
typedef struct base_struct *GTY((callback)) (*allocator_func)(
  size_t size, 
  void *context
);

/* TYPE_LANG_STRUCT: GCC internal/lang-specific types */

/* Vector type (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

/* Simulated tree node structure (like GCC's tree_node) */
struct GTY(()) tree_common {
  enum tree_code code;
  union tree_node *chain;
};

union GTY((desc ("%h.code"))) tree_node {
  struct tree_common common;
  /* Various tree node types would go here */
};

/* Simulated RTL structure */
struct GTY(()) rtx_def {
  int code;
  union {
    int rt_int;
    char *rt_str;
  } u;
};

typedef struct rtx_def *rtx;

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
  /* Struct field */
  struct base_struct embedded;
  
  /* Union field */
  union my_union choice;
  
  /* Pointer field */
  struct container_struct *next;
  
  /* Array field */
  int counts[10];
  
  /* Pointer to array */
  int *dynamic_array;
  
  /* Function pointer */
  compare_func comparator;
  
  /* String field */
  char *description;
  
  /* Scalar fields */
  enum color color;
  _Bool valid;
  
  /* Nested struct */
  struct {
    int x;
    int y;
  } point;
};

/* Recursive type structure */
struct GTY(()) tree_node_struct {
  int value;
  struct tree_node_struct *GTY((skip)) left;
  struct tree_node_struct *GTY((skip)) right;
  char *GTY((tag ("0"))) data;
};

/* Type with variable arguments in callback */
typedef int GTY((callback)) (*varargs_func)(int, ...);

/* Opaque pointer type (pointer to undefined struct) */
typedef struct opaque_undefined_type *opaque_ptr;

/* Array of function pointers */
typedef void (*func_ptr_array[5])(void);

/* Union containing struct with array */
union GTY(()) complex_union {
  struct {
    int header;
    char data[100];
  } data_block;
  struct {
    float x, y, z;
  } coordinates;
};

/* Type definition chain */
typedef int basic_int;
typedef basic_int wrapped_int;
typedef wrapped_int double_wrapped_int;

/* Incomplete array in struct */
struct GTY(()) flexible_struct {
  int length;
  char data[];
};

#endif /* TEST_GTYPE_COVERAGE_H */
