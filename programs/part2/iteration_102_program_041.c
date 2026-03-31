#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*simple_callback)(int) GTY((callback));

/* Another callback type with parameters */
typedef int (*complex_callback)(const char*, void*) GTY((callback));

/* TYPE_STRUCT with callback field */
struct GTY(()) callback_container {
  simple_callback cb1;
  complex_callback cb2;
  int id;
};

/* TYPE_UNION containing callback */
union GTY(()) callback_union {
  simple_callback fn;
  int value;
  void* ptr;
};

/* TYPE_ARRAY of callbacks */
struct GTY(()) callback_array {
  simple_callback handlers[4];
  int count;
};

/* Nested struct with callback */
struct GTY(()) outer_struct {
  struct callback_container container;
  int flags;
};

/* TYPE_POINTER to callback type */
typedef simple_callback* callback_ptr GTY((tag("CALLBACK_PTR")));

/* Mixed struct with multiple types */
struct GTY(()) mixed_types {
  /* TYPE_SCALAR */
  int scalar_field;
  
  /* TYPE_STRING */
  const char* GTY((tag("STRING_FIELD"))) name;
  
  /* TYPE_POINTER */
  struct outer_struct* next;
  
  /* TYPE_CALLBACK */
  simple_callback notify;
  
  /* TYPE_ARRAY */
  int numbers[8];
};

/* Callback in typedef struct */
typedef struct GTY(()) {
  simple_callback handler;
  int priority;
} callback_wrapper;

/* Union with callback and struct */
union GTY(()) complex_union {
  struct mixed_types as_struct;
  callback_wrapper as_wrapper;
  simple_callback as_callback;
};

#endif /* CALLBACK_TEST_H */
