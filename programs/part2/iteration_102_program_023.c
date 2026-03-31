#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with GTY((callback)) annotation */
typedef void (*callback_func)(int) GTY((callback));

/* TYPE_STRUCT: Simple struct with callback field */
struct GTY(()) callback_container {
  callback_func handler;
  int id;
};

/* TYPE_UNION: Union containing a callback */
union GTY(()) callback_union {
  callback_func fn;
  int value;
  void* GTY((skip)) ptr;
};

/* TYPE_ARRAY: Array of callbacks */
struct GTY(()) callback_array {
  callback_func handlers[4];
  int count;
};

/* Nested callback structure */
struct GTY(()) nested_callback {
  struct callback_container container;
  callback_func extra_handler;
};

/* Callback with parameters */
typedef int (*complex_callback)(void*, int, const char*) GTY((callback));

struct GTY(()) complex_struct {
  complex_callback process;
  void* GTY((skip)) user_data;
};

#endif /* CALLBACK_TEST_H */
