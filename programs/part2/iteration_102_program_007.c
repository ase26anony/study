/* gty-test-callbacks.h - Test header for gengtype TYPE_CALLBACK coverage */
#ifndef GTY_TEST_CALLBACKS_H
#define GTY_TEST_CALLBACKS_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*process_fn)(const char*, size_t) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_POINTER: Pointer to simple struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_UNION: Union containing various types */
union GTY(()) my_union {
  int int_val;
  simple_ptr ptr_val;
  float float_val;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) with_array {
  int arr[10];
  char name[32];
};

/* TYPE_SCALAR: Scalar typedef with length marker */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer type */
typedef const char* my_string GTY((length));

/* TYPE_CALLBACK inside TYPE_STRUCT: Struct containing callback pointer */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  process_fn processor;
  int id;
};

/* TYPE_CALLBACK inside TYPE_UNION: Union with callback alternative */
union GTY(()) callback_union {
  simple_callback_fn fn_ptr;
  process_fn proc_ptr;
  int fallback;
};

/* TYPE_ARRAY of TYPE_CALLBACK: Array of callbacks */
struct GTY(()) callback_array {
  simple_callback_fn handlers[5];
  int count;
};

/* TYPE_LANG_STRUCT: Language-specific structure (simulated) */
struct GTY(()) lang_specific {
  int lang_tag;
  void* GTY((skip)) lang_data;  /* Skip this for GC */
  simple_callback_fn lang_callback;
};

/* Nested structures with callbacks */
struct GTY(()) outer_struct {
  callback_container container;
  my_union data;
  with_array arrays;
};

/* TYPE_USER_STRUCT: Forward declared struct */
struct GTY(()) user_defined;
typedef struct user_defined user_defined_t;

/* Callback that uses user-defined struct */
typedef void (*user_callback_fn)(user_defined_t*) GTY((callback));

struct GTY(()) user_defined {
  int id;
  user_callback_fn notify;
  simple_callback_fn cleanup;
};

/* Complex nested example with multiple callback types */
struct GTY(()) complex_example {
  /* Direct callback field */
  simple_callback_fn direct_cb;
  
  /* Union containing callbacks */
  callback_union cb_union;
  
  /* Array of callback containers */
  callback_container containers[3];
  
  /* Pointer to callback */
  simple_callback_fn* cb_ptr GTY((tag("CB_PTR")));
  
  /* String field */
  my_string description;
};

/* TYPE_CALLBACK with struct parameter */
struct GTY(()) param_struct {
  int x, y;
};

typedef void (*struct_callback_fn)(param_struct*) GTY((callback));

/* Final struct using all types */
struct GTY(()) master_type {
  simple_struct basic;
  my_union choice;
  with_array data;
  callback_container cb_container;
  callback_union cb_union;
  lang_specific lang;
  outer_struct nested;
  user_defined_t* user_ptr GTY((tag("USER_PTR")));
  complex_example complex;
  struct_callback_fn struct_cb;
  my_scalar scalar_val;
  my_string str_val;
};

#endif /* GTY_TEST_CALLBACKS_H */
