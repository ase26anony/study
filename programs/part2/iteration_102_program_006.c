#ifndef GTY_CALLBACK_TEST_H
#define GTY_CALLBACK_TEST_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Basic callback function pointer type
 * This will create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int) GTY((callback));

/*
 * Another callback type with different signature
 */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/*
 * TYPE_STRUCT: Simple struct type
 */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/*
 * TYPE_UNION: Union type
 */
union GTY(()) my_union {
  int int_val;
  void* ptr_val;
  double double_val;
};

/*
 * TYPE_POINTER: Pointer type with tag
 */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/*
 * TYPE_ARRAY: Struct containing array
 */
struct GTY(()) array_container {
  int values[20];
  char name[50];
};

/*
 * TYPE_SCALAR: Scalar typedef with length attribute
 */
typedef unsigned long my_scalar GTY((length));

/*
 * TYPE_STRING: String pointer type
 */
typedef const char* my_string GTY((length));

/*
 * Nested callback structure: Struct containing callback function pointers
 * This ensures TYPE_CALLBACK is discovered within a larger structure
 */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn handler GTY((skip));
  
  /* Array of callbacks */
  complex_callback_fn callbacks[5];
  
  /* Pointer to struct with callback */
  struct_ptr next;
  
  /* Union containing callback */
  union GTY(()) {
    simple_callback_fn cb_fn;
    int cb_id;
  } callback_union;
};

/*
 * More complex nested structure with multiple callback levels
 */
struct GTY(()) nested_callbacks {
  /* Container with callbacks */
  callback_container* container GTY((tag("CALLBACK_CONTAINER")));
  
  /* Direct callback array */
  simple_callback_fn direct_handlers[3];
  
  /* Union that can hold either callback or data */
  union GTY(()) {
    simple_callback_fn handler;
    complex_callback_fn complex_handler;
    void* data;
  } multi_union;
  
  /* Struct with inline callback definition */
  struct GTY(()) {
    int id;
    void (*inline_callback)(void*) GTY((callback));
  } inline_struct;
};

/*
 * TYPE_USER_STRUCT: Forward declared struct that will be user-defined
 */
struct GTY(()) user_defined_struct;

/*
 * Complete definition of user-defined struct with callback
 */
struct GTY(()) user_defined_struct {
  int magic_number;
  simple_callback_fn user_callback;
  user_defined_struct* next;
};

/*
 * Union containing various callback types
 */
union GTY(()) callback_variant {
  simple_callback_fn simple;
  complex_callback_fn complex;
  void (*another_callback)(double) GTY((callback));
};

/*
 * Struct with multiple callback types mixed with other fields
 */
struct GTY(()) mixed_types {
  /* Scalar fields */
  int count;
  my_scalar length;
  
  /* String field */
  my_string description;
  
  /* Callback fields */
  simple_callback_fn on_start;
  complex_callback_fn on_data;
  
  /* Array field */
  int numbers[10];
  
  /* Pointer to union containing callback */
  callback_variant* variant GTY((tag("VARIANT_PTR")));
  
  /* Nested struct with callback */
  struct GTY(()) {
    int id;
    void (*nested_cb)(int, const char*) GTY((callback));
  } nested;
};

/*
 * Global callback variable declaration (extern)
 */
extern simple_callback_fn global_callback_handler GTY((callback));

/*
 * Function pointer type without callback marker (should not create TYPE_CALLBACK)
 * This is for contrast to ensure only GTY((callback)) creates the type
 */
typedef void (*regular_function_ptr)(int);

/*
 * Struct using regular function pointer (not a callback type)
 */
struct GTY(()) no_callback_struct {
  regular_function_ptr regular_fn;  /* This won't create TYPE_CALLBACK */
  int data;
};

#endif /* GTY_CALLBACK_TEST_H */
