#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*complex_callback_fn)(const char*, void*) GTY((callback));

/* TYPE_STRUCT: Plain struct with scalar field */
struct GTY(()) simple_struct {
  int field;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int a;
  void* GTY((skip)) b;
  simple_callback_fn fn;  /* Contains callback type */
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array type within a struct */
struct GTY(()) array_container {
  int arr[10];
  simple_callback_fn callbacks[5];  /* Array of callbacks */
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* Nested struct containing callback */
struct GTY(()) callback_container {
  simple_callback_fn handler;
  complex_callback_fn processor;
  struct_ptr next;
};

/* Union mixing callback with other types */
union GTY(()) mixed_union {
  simple_callback_fn callback;
  my_scalar value;
  struct_ptr ptr;
};

/* Struct with multiple callback fields */
struct GTY(()) multi_callback_struct {
  simple_callback_fn start_fn;
  complex_callback_fn process_fn;
  void (*unmarked_fn)(void);  /* Not GTY marked */
  array_container container;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_specific {
  int lang_data;
  simple_callback_fn lang_callback;
};

/* Callback with struct parameter */
struct GTY(()) param_struct {
  int id;
  char name[32];
};

typedef void (*struct_callback_fn)(param_struct*) GTY((callback));

/* Container using struct-parameter callback */
struct GTY(()) uses_struct_callback {
  struct_callback_fn handler;
  param_struct data;
};

#endif /* TEST_CALLBACK_TYPES_H */
