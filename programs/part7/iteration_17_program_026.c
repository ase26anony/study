/* gtype-test.cc - Comprehensive type test for gengtype coverage */
/* This file should be placed in the gcc/ directory and built as part of GCC */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Forward declarations (TYPE_UNDEFINED) */
struct opaque_struct;
union opaque_union;

/* Scalar types (TYPE_SCALAR) */
GTY(()) int global_int;
GTY(()) float global_float;
GTY(()) double global_double;
GTY(()) char global_char;

/* String type (TYPE_STRING) */
GTY(()) const char *global_string = "test string";

/* Basic struct (TYPE_STRUCT) */
struct GTY(()) basic_struct {
  int a;
  float b;
};

/* User struct via typedef (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
  int x;
  double y;
} user_struct_t;

/* Union type (TYPE_UNION) */
union GTY(()) basic_union {
  int as_int;
  float as_float;
  struct basic_struct *GTY((tag("0"))) as_struct_ptr;
};

/* Pointer types (TYPE_POINTER) */
GTY(()) int *int_ptr;
GTY(()) struct basic_struct *struct_ptr;
GTY(()) user_struct_t *user_struct_ptr;
GTY(()) union basic_union *union_ptr;

/* Array types (TYPE_ARRAY) */
GTY(()) int int_array[10];
GTY(()) struct basic_struct struct_array[5];
GTY(()) user_struct_t *ptr_array[8];

/* Incomplete array */
struct GTY(()) with_incomplete_array {
  int count;
  int items[];
};

/* Callback/function pointer types (TYPE_CALLBACK) */
typedef int (*compare_func_t)(const void *, const void *);
typedef void (*callback_t)(int, float);

struct GTY(()) with_callback {
  compare_func_t GTY((skip)) compare;
  callback_t GTY((skip)) callback;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
  /* Array of pointers to unions */
  union basic_union *GTY((length("count"))) union_array[10];
  
  /* Pointer to array */
  int (*GTY((skip)) matrix_ptr)[5][5];
  
  /* Struct containing callback */
  struct with_callback GTY((skip)) callback_struct;
  
  /* Self-referential pointer */
  struct complex_nested *GTY((skip)) next;
};

/* Language-specific struct (TYPE_LANG_STRUCT) */
/* Using GCC attributes to potentially create language-specific types */
struct GTY(()) lang_struct 
#ifdef __cplusplus
  __attribute__((transaction_safe))
#endif
{
  int transaction_id;
  void *GTY((skip)) transaction_data;
};

/* Union containing mixed types */
union GTY(()) mixed_union {
  struct basic_struct as_struct;
  user_struct_t as_user_struct;
  struct with_callback as_callback;
  struct lang_struct as_lang_struct;
};

/* Function pointer returning pointer to array */
typedef int (*complex_func_t)(void)[10];

/* Another complex type: struct with function pointer returning struct pointer */
struct GTY(()) node;
typedef struct node *(*allocator_t)(int size);

struct GTY(()) node {
  int value;
  struct node *GTY((skip)) left;
  struct node *GTY((skip)) right;
  allocator_t GTY((skip)) allocator;
};

/* Global instances to ensure types are used */
GTY(()) struct basic_struct global_basic_struct = {1, 2.0f};
GTY(()) user_struct_t global_user_struct = {10, 20.5};
GTY(()) union basic_union global_union;
GTY(()) struct complex_nested *global_complex = NULL;
GTY(()) struct lang_struct global_lang_struct = {0, NULL};
GTY(()) union mixed_union global_mixed_union;

/* Function using the types to prevent dead code elimination */
void GTY((skip)) gt_test_function(void)
{
  /* Use scalar types */
  global_int = 42;
  global_float = 3.14f;
  
  /* Use string */
  const char *local_str = global_string;
  
  /* Use structs */
  global_basic_struct.a = 100;
  global_user_struct.x = 200;
  
  /* Use arrays */
  for (int i = 0; i < 10; i++) {
    int_array[i] = i;
  }
  
  /* Use pointers */
  if (int_ptr) {
    *int_ptr = 50;
  }
  
  /* Initialize union */
  global_union.as_int = 1234;
  
  /* Initialize lang struct */
  global_lang_struct.transaction_id = 999;
  
  /* Initialize mixed union */
  global_mixed_union.as_struct.a = 1;
  global_mixed_union.as_struct.b = 2.0f;
}

/* Additional test structures with various GTY annotations */
struct GTY(()) gty_annotated {
  /* Skip annotation */
  int *GTY((skip)) skipped_ptr;
  
  /* Length annotation */
  int *GTY((length("len"))) array_ptr;
  size_t len;
  
  /* Nested pointers */
  struct gty_annotated *GTY((skip)) next;
  struct gty_annotated **GTY((skip)) prev_ptr;
};

/* Test for callback with complex return type */
typedef struct GTY(()) callback_return {
  int status;
  char *GTY((skip)) message;
} callback_return_t;

typedef callback_return_t *(*complex_callback_t)(int param);

struct GTY(()) has_complex_callback {
  complex_callback_t GTY((skip)) handler;
  int id;
};

/* Main function for standalone compilation */
#ifndef IN_GCC
int main(void)
{
  gt_test_function();
  return 0;
}
#endif
