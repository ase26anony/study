#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Basic callback function pointer type
 * This should create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/*
 * Another callback type with different signature
 */
typedef int (*process_data_fn)(const char*, size_t) GTY((callback));

/* 
 * TYPE_SCALAR: Simple scalar type
 */
typedef unsigned long my_scalar GTY((length));

/*
 * TYPE_STRUCT: Simple struct type
 */
struct GTY(()) simple_struct {
  int field1;
  my_scalar field2;
};

/*
 * TYPE_POINTER: Pointer type to simple_struct
 */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/*
 * TYPE_UNION: Union type
 */
union GTY(()) data_union {
  int int_val;
  void* ptr_val;
  my_scalar scalar_val;
};

/*
 * TYPE_ARRAY: Struct containing array
 */
struct GTY(()) array_container {
  int values[20];
  simple_callback_fn callbacks[5];
};

/*
 * Nested callback structure - callback inside a struct
 * This tests that gengtype traverses and finds the callback type
 */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn handler GTY((tag("HANDLER")));
  
  /* Array of callbacks */
  process_data_fn processors[3];
  
  /* Union containing callback */
  union GTY(()) {
    simple_callback_fn cb;
    int id;
  } callback_union;
  
  /* Pointer to struct with callback */
  struct GTY(()) nested {
    simple_callback_fn nested_cb;
    int data;
  }* nested_ptr;
};

/*
 * Complex structure mixing all types including callbacks
 */
struct GTY(()) complex_type {
  /* TYPE_STRUCT member */
  simple_struct base;
  
  /* TYPE_UNION member */
  data_union storage;
  
  /* TYPE_ARRAY member */
  array_container arrays;
  
  /* TYPE_CALLBACK member */
  process_data_fn data_processor;
  
  /* TYPE_POINTER to callback */
  simple_callback_fn* callback_ptr;
  
  /* Nested callback container */
  callback_container container;
};

/*
 * TYPE_USER_STRUCT: Forward declared struct with callback
 */
struct GTY(()) user_defined;
typedef struct user_defined user_defined_t;

struct GTY(()) user_defined {
  simple_callback_fn init;
  user_defined_t* next;
};

/*
 * Callback type used in a linked list structure
 */
typedef void (*list_traverse_fn)(void* data) GTY((callback));

struct GTY(()) list_node {
  void* data;
  list_traverse_fn traverse;
  struct list_node* GTY((skip)) next;
};

/*
 * Union specifically for testing TYPE_CALLBACK in unions
 */
union GTY(()) callback_test_union {
  simple_callback_fn fn1;
  process_data_fn fn2;
  list_traverse_fn fn3;
  int discriminator;
};

/*
 * Structure with multiple callback types for comprehensive testing
 */
struct GTY(()) multi_callback_test {
  /* Different callback signatures */
  simple_callback_fn cb_simple;
  process_data_fn cb_process;
  list_traverse_fn cb_traverse;
  
  /* Callback in array */
  simple_callback_fn cb_array[4];
  
  /* Pointer to callback */
  process_data_fn* cb_ptr;
  
  /* Callback in nested struct */
  struct GTY(()) {
    simple_callback_fn nested_callback;
  } inner;
};

#endif /* TEST_CALLBACK_TYPES_H */
