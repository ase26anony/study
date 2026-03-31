#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Basic callback function pointer type
 * This should create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 
 * Another callback type with different signature
 */
typedef int (*complex_callback_fn)(void*, size_t) GTY((callback));

/* 
 * TYPE_STRUCT: Simple struct type
 */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* 
 * TYPE_UNION: Simple union type
 */
union GTY(()) simple_union {
  int int_val;
  void* ptr_val;
  double double_val;
};

/* 
 * TYPE_POINTER: Pointer type with tag
 */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 
 * TYPE_ARRAY: Struct containing an array
 */
struct GTY(()) array_container {
  int data[20];
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
 * Nested callback structure: Struct containing callback pointers
 * This ensures TYPE_CALLBACK is discovered within a TYPE_STRUCT
 */
struct GTY(()) callback_container {
  /* Callback function pointer field */
  simple_callback_fn handler GTY((tag("SIMPLE_HANDLER")));
  
  /* Array of callback pointers */
  complex_callback_fn handlers[3];
  
  /* Regular data field */
  int id;
};

/* 
 * Union containing a callback type
 * Tests TYPE_CALLBACK within TYPE_UNION
 */
union GTY(()) callback_union {
  simple_callback_fn callback;
  int token;
  void* data;
};

/* 
 * More complex nested structure with multiple callback types
 */
struct GTY(()) nested_callbacks {
  /* Direct callback field */
  simple_callback_fn primary_handler;
  
  /* Struct containing callback */
  callback_container container;
  
  /* Union containing callback */
  callback_union union_field;
  
  /* Pointer to callback type */
  complex_callback_fn* handler_ptr;
};

/* 
 * Callback with user-defined struct parameter
 */
struct GTY(()) user_data {
  int id;
  char* name;
};

typedef void (*data_callback_fn)(user_data*) GTY((callback));

/* 
 * Struct using the data callback
 */
struct GTY(()) data_processor {
  data_callback_fn processor;
  user_data* data;
};

/* 
 * Array of callbacks
 */
typedef simple_callback_fn callback_array[5] GTY((tag("CALLBACK_ARRAY")));

/* 
 * Pointer to array of callbacks
 */
typedef callback_array* callback_array_ptr GTY((tag("CALLBACK_ARRAY_PTR")));

/* 
 * Struct with all types mixed together
 */
struct GTY(()) mega_struct {
  /* TYPE_STRUCT field */
  simple_struct basic;
  
  /* TYPE_UNION field */
  simple_union choice;
  
  /* TYPE_POINTER field */
  struct_ptr ptr;
  
  /* TYPE_ARRAY field */
  array_container arrays;
  
  /* TYPE_CALLBACK field */
  simple_callback_fn callback;
  
  /* TYPE_SCALAR field */
  my_scalar count;
  
  /* TYPE_STRING field */
  my_string description;
  
  /* Nested callback container */
  callback_container container;
};

#endif /* TEST_CALLBACK_TYPES_H */
