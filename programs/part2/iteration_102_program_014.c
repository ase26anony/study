#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*process_fn)(const char*, size_t) GTY((callback));

/* TYPE_STRUCT: Simple struct with scalar field */
struct GTY(()) simple_struct {
  int field1;
  long field2;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_UNION: Union type */
union GTY(()) data_union {
  int int_val;
  void* ptr_val;
  double dbl_val;
};

/* TYPE_ARRAY: Struct containing array */
struct GTY(()) array_container {
  int numbers[20];
  char name[50];
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* TYPE_CALLBACK nested in struct */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  process_fn processor;
  int id;
};

/* TYPE_CALLBACK in array within struct */
struct GTY(()) multi_handler {
  simple_callback_fn handlers[5];
  int handler_count;
};

/* TYPE_CALLBACK in union */
union GTY(()) callback_or_data {
  simple_callback_fn callback;
  void* data;
  int tag;
};

/* Complex nested structure with callback */
struct GTY(()) nested_callback {
  struct GTY(()) inner {
    simple_callback_fn cb;
    int priority;
  } inner_struct;
  
  callback_container* GTY((tag("CC_PTR"))) container_ptr;
  data_union storage;
};

/* TYPE_CALLBACK with struct parameter */
struct GTY(()) param_struct {
  int x, y;
};

typedef void (*struct_callback_fn)(param_struct*) GTY((callback));

/* Final struct using all types */
struct GTY(()) master_type {
  simple_callback_fn master_callback;
  nested_callback nested;
  array_container arrays;
  my_scalar count;
  my_string description;
  callback_or_data choice;
};

#endif /* TEST_CALLBACK_TYPES_H */
