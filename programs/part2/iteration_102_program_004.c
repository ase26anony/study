#ifndef TEST_GTY_CALLBACKS_H
#define TEST_GTY_CALLBACKS_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Basic callback function pointer type with GTY((callback))
 * This should create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int, void*) GTY((callback));

/*
 * Another callback type with different signature
 */
typedef int (*process_data_fn)(const char*, size_t) GTY((callback));

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
  int as_int;
  void* as_ptr;
  double as_double;
};

/* 
 * TYPE_POINTER: Pointer type with tag
 */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* 
 * TYPE_ARRAY: Struct containing an array
 */
struct GTY(()) with_array {
  int arr[10];
  char name[32];
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
 * This ensures the callback type is referenced within a GTY-annotated struct
 */
struct GTY(()) callback_container {
  /* Callback function pointer field */
  simple_callback_fn handler GTY((tag("CALLBACK_FIELD")));
  
  /* Array of callbacks */
  process_data_fn processors[3];
  
  /* Regular fields */
  int id;
  struct_ptr next;
};

/* 
 * Union containing a callback
 */
union GTY(()) callback_union {
  simple_callback_fn callback;
  int callback_id;
  void* user_data;
};

/* 
 * More complex structure with multiple callback types
 */
struct GTY(()) complex_system {
  /* Direct callback */
  simple_callback_fn notify;
  
  /* Union with callback */
  callback_union union_field;
  
  /* Container with callback */
  callback_container container;
  
  /* Array of callback containers */
  callback_container containers[5];
  
  /* Pointer to callback function */
  process_data_fn* processor_ptr;
};

/* 
 * Callback used in a typedef struct pattern
 */
typedef struct GTY(()) linked_node {
  int data;
  simple_callback_fn on_update;
  struct linked_node* GTY((skip)) next;  /* skip prevents infinite recursion */
} linked_node;

/* 
 * Function pointer type without callback marker (for contrast)
 * This should NOT create TYPE_CALLBACK
 */
typedef void (*regular_func_ptr)(void);

/* 
 * Struct mixing regular and callback function pointers
 */
struct GTY(()) mixed_pointers {
  /* This is a callback (has GTY((callback)) in its typedef) */
  simple_callback_fn callback_member;
  
  /* This is a regular function pointer (no GTY annotation) */
  regular_func_ptr regular_member;
  
  /* This is a pointer to callback type */
  process_data_fn* callback_ptr;
};

/* 
 * Callback with struct parameter containing callbacks (nested case)
 */
struct GTY(()) nested_callback_param {
  int value;
  simple_callback_fn action;
};

typedef void (*nested_callback_fn)(nested_callback_param*) GTY((callback));

struct GTY(()) uses_nested_callback {
  nested_callback_fn processor;
  nested_callback_param param;
};

#endif /* TEST_GTY_CALLBACKS_H */
