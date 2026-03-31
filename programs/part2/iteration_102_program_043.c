#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with parameters */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Simple struct */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_UNION: Simple union */
union GTY(()) simple_union {
  int int_val;
  void* ptr_val;
  double double_val;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array within a struct */
struct GTY(()) array_container {
  int data[20];
  simple_callback_fn init_fn;
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* TYPE_CALLBACK inside struct: Nested callback structure */
struct GTY(()) callback_container {
  /* Callback function pointer field */
  simple_callback_fn notify GTY((tag("NOTIFY_CB")));
  
  /* Array of callbacks */
  process_callback_fn handlers[4];
  
  /* Regular fields */
  int id;
  struct_ptr next;
};

/* TYPE_UNION with callback */
union GTY(()) mixed_union {
  simple_callback_fn callback;
  int counter;
  my_string name;
};

/* Complex nested structure with multiple callback types */
struct GTY(()) complex_wrapper {
  /* Direct callback field */
  process_callback_fn processor;
  
  /* Struct containing callback */
  callback_container container;
  
  /* Union with callback */
  mixed_union choice;
  
  /* Array of structs with callbacks */
  callback_container items[5];
  
  /* Pointer to callback */
  simple_callback_fn* callback_ptr;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_specific {
  int lang_code;
  void* lang_data;
  simple_callback_fn lang_callback;
};

/* Another callback type for chaining */
typedef void (*chain_callback_fn)(complex_wrapper*, void*) GTY((callback));

/* Final structure using all types */
struct GTY(()) master_structure {
  simple_struct base;
  array_container arrays;
  complex_wrapper wrapper;
  chain_callback_fn chain;
  lang_specific lang_info;
  my_string description;
};

#endif /* CALLBACK_TEST_H */
