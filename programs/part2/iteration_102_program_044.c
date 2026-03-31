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

/* Union with callback alternative */
union GTY(()) callback_or_int {
  simple_callback_fn fn;
  int value;
};

/* Array of callbacks */
struct GTY(()) callback_array {
  simple_callback_fn handlers[3];
  int count;
};

#endif /* CALLBACK_TEST_1_H */
