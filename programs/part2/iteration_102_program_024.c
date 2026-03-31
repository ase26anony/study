#ifndef CALLBACK_TEST_1_H
#define CALLBACK_TEST_1_H

#include "config.h"
#include "system.h"

/* Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Struct containing a callback */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  int id;
};

/* Union mixing callback with other types */
union GTY(()) callback_mixed_union {
  simple_callback_fn fn;
  void* GTY((tag("PTR"))) ptr;
  int value;
};

/* Array of callbacks */
struct GTY(()) callback_array_struct {
  simple_callback_fn handlers[3];
  int count;
};

#endif /* CALLBACK_TEST_1_H */
