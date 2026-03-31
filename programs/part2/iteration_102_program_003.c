/* Header file with GTY annotations to test gengtype coverage */
#ifndef TEST_GTY_CALLBACKS_H
#define TEST_GTY_CALLBACKS_H

/* Include necessary GCC headers for proper parsing */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Core callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
  int field1;
  long field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined handling */
struct GTY((user)) user_struct {
  void* GTY((skip)) opaque_data;
  int id;
};

/* TYPE_UNION: Union type */
union GTY(()) simple_union {
  int int_val;
  void* ptr_val;
  double double_val;
};

/* TYPE_POINTER: Pointer type with tag */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) array_container {
  int numbers[20];
  char name[50];
};

/* TYPE_SCALAR: Scalar typedef with length attribute */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String type */
typedef const char* my_string GTY((length));

/* Nested callback structure - callback inside a struct */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  process_callback_fn processor;
  int callback_count;
};

/* Array of callbacks */
struct GTY(()) multi_callback_container {
  simple_callback_fn handlers[5];
  int active_handlers;
};

/* Union containing callback */
union GTY(()) callback_union {
  simple_callback_fn callback;
  int callback_id;
  void* alternate;
};

/* More complex nested structure with callback */
struct GTY(()) complex_struct {
  struct_ptr next;
  array_container data;
  callback_container callbacks;
  my_string description;
  my_scalar counter;
};

/* Callback used in a typedef struct */
typedef struct GTY(()) typed_callback_struct {
  process_callback_fn main_callback;
  simple_callback_fn fallback;
  union GTY(()) {
    int mode;
    simple_callback_fn mode_callback;
  } selector;
} typed_callback_struct_t;

/* Another callback type for chaining */
typedef void (*chain_callback_fn)(typed_callback_struct_t*) GTY((callback));

/* Struct using chain callback */
struct GTY(()) chain_container {
  chain_callback_fn chain_start;
  chain_callback_fn chain_end;
  typed_callback_struct_t* elements;
};

/* Mixed types in one header to ensure all switch cases are hit */
struct GTY(()) master_container {
  simple_struct basic;
  user_struct user;
  simple_union choice;
  array_container items;
  callback_container handlers;
  complex_struct* complex;
  my_string title;
  my_scalar total;
};

#endif /* TEST_GTY_CALLBACKS_H**
