#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with GTY((callback)) annotation */
typedef void (*callback_func)(int) GTY((callback));

/* TYPE_STRUCT: Simple struct with callback field */
struct GTY(()) struct_with_callback {
  callback_func handler;
  int data;
};

/* TYPE_UNION: Union containing a callback */
union GTY(()) union_with_callback {
  callback_func fn;
  int id;
  void* ptr;
};

/* TYPE_ARRAY: Array of callbacks */
struct GTY(()) struct_with_callback_array {
  callback_func handlers[4];
  int count;
};

/* Nested callback structure */
typedef int (*nested_callback)(void*, int) GTY((callback));

struct GTY(()) nested_container {
  nested_callback process;
  struct_with_callback* next GTY((skip));
};

/* Callback in a pointer context */
struct GTY(()) callback_wrapper {
  callback_func* callbacks GTY((length("count")));
  int count;
};

/* Multiple callback types in one struct */
typedef void (*start_callback)(void) GTY((callback));
typedef void (*end_callback)(int result) GTY((callback));

struct GTY(()) multi_callback_struct {
  start_callback begin;
  end_callback finish;
  int state;
};

#endif /* CALLBACK_TEST_H */
