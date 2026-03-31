#ifndef TEST_CALLBACK_H
#define TEST_CALLBACK_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with different signature */
typedef void* (*alloc_callback_fn)(size_t, void*) GTY((callback));

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

/* TYPE_ARRAY: Struct with array */
struct GTY(()) array_struct {
  int values[10];
  simple_callback_fn callbacks[5];
};

/* TYPE_SCALAR: Scalar typedef with length marker */
typedef unsigned long my_scalar GTY((length));

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
  /* This field contains a TYPE_CALLBACK */
  simple_callback_fn handler;
  
  /* Regular fields */
  int id;
  struct_ptr next;
};

/* More complex: struct with multiple callbacks */
struct GTY(()) multi_callback_struct {
  simple_callback_fn start_cb;
  alloc_callback_fn alloc_cb;
  simple_callback_fn end_cb;
  
  /* Union containing a callback */
  union GTY(()) {
    simple_callback_fn fn_ptr;
    int mode;
  } GTY((tag("CALLBACK_UNION"))) callback_union;
};

/* TYPE_USER_STRUCT: Forward declared struct */
struct GTY(()) forward_declared;
struct GTY(()) forward_declared {
  simple_callback_fn callback;
  forward_declared* next;
};

/* Callback in a union */
union GTY(()) callback_in_union {
  simple_callback_fn callback;
  alloc_callback_fn allocator;
  int token;
};

/* Array of callbacks */
typedef simple_callback_fn callback_array[10] GTY((tag("CALLBACK_ARRAY")));

/* Struct with array of callbacks */
struct GTY(()) struct_with_callback_array {
  callback_array handlers;
  int count;
};

/* Complex nested structure with callbacks */
struct GTY(()) outer_container {
  /* Inner struct with callback */
  struct GTY(()) inner {
    simple_callback_fn process;
    int state;
  } GTY((tag("INNER_STRUCT"))) processor;
  
  /* Union with callback option */
  union GTY(()) {
    simple_callback_fn direct;
    struct_ptr indirect;
  } GTY((tag("ACTION_UNION"))) action;
  
  /* Array of callback containers */
  callback_container containers[5];
};

#endif /* TEST_CALLBACK_H */
