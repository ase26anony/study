#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*simple_callback)(int) GTY((callback));

/* Another callback type with parameters */
typedef int (*complex_callback)(const char*, void*) GTY((callback));

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

/* TYPE_POINTER: Typedef pointer */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Struct with array */
struct GTY(()) array_struct {
  int numbers[10];
  char name[32];
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
  simple_callback handler1;
  complex_callback handler2;
  int id;
};

/* Array of callbacks */
struct GTY(()) callback_array {
  simple_callback handlers[5];
  int count;
};

/* Union containing callback */
union GTY(()) callback_union {
  simple_callback fn_ptr;
  int (*regular_fn)(void);  /* Not a GTY callback */
  int identifier;
};

/* Complex nested structure with callback */
struct GTY(()) nested_callback {
  struct GTY(()) inner {
    simple_callback cb;
    int data;
  } inner_struct;
  
  callback_container* GTY((skip)) next;
  int flags;
};

/* Callback in a typedef struct */
typedef struct GTY(()) {
  simple_callback start_fn;
  complex_callback process_fn;
  void (*cleanup_fn)(void);  /* Not GTY annotated */
} callback_suite;

#endif /* CALLBACK_TEST_H */
