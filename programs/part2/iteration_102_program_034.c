#ifndef CALLBACK_TEST_H
#define CALLBACK_TEST_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Function pointer with callback marker */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* Another callback type with parameters */
typedef int (*complex_callback_fn)(const char *, void *) GTY((callback));

/* TYPE_STRUCT: Simple struct */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_UNION: Simple union */
union GTY(()) simple_union {
  int int_val;
  void *ptr_val;
  simple_callback_fn callback_val;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct *struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array within a struct */
struct GTY(()) array_container {
  int numbers[10];
  simple_callback_fn callbacks[5];
};

/* TYPE_USER_STRUCT: Forward declaration */
struct GTY(()) user_struct;

/* Nested callback structure - callback inside struct */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn handler GTY((skip));
  
  /* Array of callbacks */
  complex_callback_fn handlers[3];
  
  /* Pointer to another struct containing callback */
  struct callback_container *next;
};

/* Union containing callback */
union GTY(()) mixed_union {
  simple_callback_fn fn_ptr;
  int id;
  struct array_container *container;
};

/* Struct with multiple callback types */
struct GTY(()) multi_callback_struct {
  /* Various callback fields */
  simple_callback_fn start_fn;
  complex_callback_fn process_fn;
  void (*cleanup_fn)(void *) GTY((callback));
  
  /* Regular fields */
  int state;
  struct multi_callback_struct *next;
};

/* TYPE_LANG_STRUCT simulation (often used for language-specific types) */
struct GTY(()) lang_specific {
  int lang_tag;
  union mixed_union data;
  simple_callback_fn lang_callback;
};

/* Callback in typedef struct */
typedef struct GTY(()) {
  int id;
  simple_callback_fn notify;
} callback_wrapper;

/* Chain of structures with callbacks */
struct GTY(()) callback_chain {
  simple_callback_fn current;
  struct callback_chain *GTY((skip)) next;
  complex_callback_fn validators[2];
};

#endif /* CALLBACK_TEST_H */
