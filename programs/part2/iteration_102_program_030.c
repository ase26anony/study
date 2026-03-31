#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Core callback type definition */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_USER_STRUCT: Forward declared struct */
struct GTY(()) user_struct;
typedef struct user_struct *user_struct_ptr;

struct GTY(()) user_struct {
  int id;
  user_struct_ptr next;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
  int int_val;
  void* GTY((skip)) ptr_val;
  double double_val;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* simple_struct_ptr GTY((tag("SIMPLE_PTR")));

/* TYPE_ARRAY: Array type */
struct GTY(()) array_container {
  int values[10];
  simple_callback_fn callbacks[5];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_type {
  int lang_specific;
  struct GTY((desc("%1.lang_specific"))) lang_type *next;
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* Nested callback structure - callback inside a struct */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  int priority;
  struct callback_container* GTY((skip)) next;
};

/* Union containing callback */
union GTY(()) callback_union {
  simple_callback_fn callback;
  int callback_id;
  void* GTY((skip)) data;
};

/* Complex structure mixing multiple types including callback */
struct GTY(()) complex_type {
  /* TYPE_STRUCT member */
  simple_struct base;
  
  /* TYPE_CALLBACK member */
  simple_callback_fn notify;
  
  /* TYPE_ARRAY member with callbacks */
  simple_callback_fn handlers[3];
  
  /* TYPE_POINTER member */
  simple_struct_ptr ptr;
  
  /* TYPE_UNION member */
  data_union storage;
  
  /* TYPE_STRING member */
  my_string name;
};

/* Another callback type with different signature */
typedef int (*filter_callback_fn)(const char*, void*) GTY((callback));

/* Struct using multiple callback types */
struct GTY(()) multi_callback {
  simple_callback_fn simple;
  filter_callback_fn filter;
  int flags;
};

/* Self-referential structure with callback */
struct GTY(()) recursive_with_callback {
  simple_callback_fn process;
  struct recursive_with_callback* GTY((skip)) child;
};

#endif /* TEST_CALLBACK_TYPES_H**
