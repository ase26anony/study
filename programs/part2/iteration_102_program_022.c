#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

/* Include necessary GCC headers for gengtype parsing */
#include "config.h"
#include "system.h"

/* 
 * TYPE_CALLBACK: Core callback function pointer type
 * This will create a TYPE_CALLBACK entry in gengtype's type table
 */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* 
 * Another callback type with different signature
 */
typedef int (*complex_callback_fn)(void*, const char*) GTY((callback));

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
  int numbers[10];
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
 * Nested callback structure: Struct containing callback function pointer
 * This ensures TYPE_CALLBACK is found during type traversal
 */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  complex_callback_fn validator;
  int id;
};

/* 
 * Array of callbacks within a struct
 */
struct GTY(()) multi_callback_container {
  simple_callback_fn handlers[5];
  int count;
};

/* 
 * Union containing callback
 */
union GTY(()) callback_union {
  simple_callback_fn fn_ptr;
  complex_callback_fn complex_fn;
  int callback_id;
};

/* 
 * More complex nested structure with multiple callback types
 */
struct GTY(()) nested_callback_struct {
  struct callback_container container;
  simple_callback_fn extra_callback;
  struct_ptr optional_data;
};

/* 
 * Self-referential structure with callback
 */
struct GTY(()) recursive_callback_struct {
  simple_callback_fn processor;
  struct recursive_callback_struct* GTY((skip)) next;  /* skip prevents infinite recursion */
  int depth;
};

/* 
 * Callback with user-defined struct parameter
 */
typedef void (*struct_callback_fn)(simple_struct*) GTY((callback));

/* 
 * Container for struct-parameter callback
 */
struct GTY(()) struct_callback_container {
  struct_callback_fn processor;
  simple_struct data;
};

/* 
 * Mixed types in union with callback
 */
union GTY(()) mixed_union {
  simple_callback_fn callback;
  array_container arrays;
  my_string str;
};

/* 
 * Typedef for callback returning pointer
 */
typedef simple_struct* (*allocator_callback_fn)(size_t) GTY((callback));

/* 
 * Memory management structure with callbacks
 */
struct GTY(()) memory_manager {
  allocator_callback_fn allocate;
  void (*deallocator)(void*) GTY((callback));
  size_t pool_size;
};

#endif /* TEST_CALLBACK_TYPES_H */
