#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_STRUCT: Simple struct with scalar field */
struct GTY(()) simple_struct {
  int field;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int a;
  void * GTY((skip)) b;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct *struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array within a struct */
struct GTY(()) with_array {
  int arr[10];
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char *my_string GTY((length));

/* Nested callback structure: Callback inside a struct */
struct GTY(()) callback_container {
  /* Callback function pointer array */
  simple_callback_fn handlers[2];
  
  /* Another callback as a direct pointer */
  simple_callback_fn default_handler;
};

/* Union containing a callback */
union GTY(()) callback_union {
  simple_callback_fn fn;
  int id;
};

/* More complex: Struct with multiple callback types */
typedef int (*int_callback_fn)(const char *) GTY((callback));

struct GTY(()) complex_callback_struct {
  simple_callback_fn void_callback;
  int_callback_fn int_callback;
  struct_ptr next;
};

/* Chain of structures with callbacks */
struct GTY(()) callback_chain {
  simple_callback_fn handler;
  struct GTY((skip)) callback_chain *next;
};

/* Callback with parameters */
typedef void (*param_callback_fn)(int, const char *, void *) GTY((callback));

/* Final test structure mixing all types */
struct GTY(()) master_test {
  simple_callback_fn cb1;
  int_callback_fn cb2;
  param_callback_fn cb3;
  my_scalar count;
  my_string name;
  with_array data;
  callback_union u;
  complex_callback_struct complex;
};

#endif /* CALLBACK_TEST_H */
