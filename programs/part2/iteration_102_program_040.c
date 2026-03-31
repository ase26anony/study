#ifndef CALLBACK_TEST_1_H
#define CALLBACK_TEST_1_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*simple_callback)(int) GTY((callback));

/* TYPE_STRUCT: Basic struct type */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_POINTER: Pointer to struct */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_CALLBACK inside TYPE_STRUCT */
struct GTY(()) callback_container {
  simple_callback handler;  /* This makes gengtype process TYPE_CALLBACK */
  struct_ptr next;
};

/* TYPE_ARRAY: Array of callbacks */
struct GTY(()) callback_array {
  simple_callback handlers[5];
  int count;
};

/* TYPE_UNION: Union containing callback */
union GTY(()) callback_union {
  simple_callback fn;
  int id;
  void* data;
};

#endif /* CALLBACK_TEST_1_H */
