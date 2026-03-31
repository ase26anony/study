#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers */
#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with parameters */
typedef int (*process_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_POINTER: Pointer type to simple_struct */
typedef simple_struct* simple_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_UNION: Union type */
union GTY(()) data_union {
  int int_val;
  simple_ptr ptr_val;
  double double_val;
};

/* TYPE_ARRAY: Struct with array field */
struct GTY(()) array_container {
  int numbers[10];
  char name[32];
};

/* TYPE_SCALAR: Scalar typedef with length attribute */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer type */
typedef const char* my_string GTY((length));

/* TYPE_CALLBACK inside TYPE_STRUCT: Struct containing callback */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  process_callback_fn processor;
  int id;
};

/* TYPE_CALLBACK inside TYPE_UNION: Union with callback */
union GTY(()) callback_union {
  simple_callback_fn fn_ptr;
  process_callback_fn proc_ptr;
  int callback_id;
};

/* TYPE_ARRAY of TYPE_CALLBACK: Array of callbacks */
struct GTY(()) callback_array {
  simple_callback_fn handlers[5];
  int count;
};

/* Nested struct with callback */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    simple_callback_fn inner_callback;
    int inner_value;
  } inner;
  
  callback_container container;
  data_union union_data;
};

/* TYPE_CALLBACK with struct parameter */
struct GTY(()) callback_param;
typedef void (*struct_callback_fn)(callback_param*) GTY((callback));

struct GTY(()) callback_param {
  struct_callback_fn notify;
  int data;
};

/* Forward declaration for mutual reference */
struct GTY(()) mutually_referencing;

/* Callback that references forward-declared struct */
typedef void (*mutual_callback_fn)(struct mutually_referencing*) GTY((callback));

struct GTY(()) mutually_referencing {
  mutual_callback_fn callback;
  int value;
  struct mutually_referencing* GTY((skip)) next;  /* Skip to avoid infinite recursion */
};

#endif /* TEST_CALLBACK_TYPES_H */
