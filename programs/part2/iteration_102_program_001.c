#ifndef TEST_CALLBACK_TYPES_H
#define TEST_CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with parameters */
typedef int (*complex_callback_fn)(void*, const char*, size_t) GTY((callback));

/* TYPE_STRUCT: Plain struct */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
  int a;
  void* GTY((skip)) b;
  double c;
};

/* TYPE_POINTER: Pointer type */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Array within a struct */
struct GTY(()) array_container {
  int values[10];
  simple_callback_fn GTY((skip)) callbacks[5];
};

/* TYPE_SCALAR: Scalar typedef */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer */
typedef const char* my_string GTY((length));

/* Nested callback structure - forces traversal to find TYPE_CALLBACK */
struct GTY(()) callback_container {
  /* Direct callback pointer */
  simple_callback_fn handler;
  
  /* Array of callbacks */
  complex_callback_fn GTY((skip)) handlers[3];
  
  /* Struct containing callback */
  struct GTY(()) nested {
    simple_callback_fn nested_handler;
    int id;
  } nested_struct;
};

/* Union containing callback */
union GTY(()) callback_union {
  simple_callback_fn fn;
  complex_callback_fn complex_fn;
  int id;
  void* ptr;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_struct {
  int lang_specific;
  simple_callback_fn lang_callback;
};

/* Another callback type for more coverage */
typedef void (*final_callback_fn)(struct callback_container*, my_scalar) GTY((callback));

/* Container using multiple callback types */
struct GTY(()) multi_callback_container {
  simple_callback_fn simple;
  complex_callback_fn complex;
  final_callback_fn final;
  
  /* Pointer to callback */
  final_callback_fn* callback_ptr;
  
  /* Union with callback */
  callback_union union_with_callback;
};

#endif /* TEST_CALLBACK_TYPES_H */
