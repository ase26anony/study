#ifndef CALLBACK_TYPES_H
#define CALLBACK_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_CALLBACK: Basic callback function pointer type */
typedef void (*simple_callback_fn)(int) GTY((callback));

/* TYPE_CALLBACK: Another callback with different signature */
typedef int (*process_data_fn)(const char*, size_t) GTY((callback));

/* TYPE_STRUCT: Simple struct type */
struct GTY(()) simple_struct {
  int field1;
  double field2;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
  int int_val;
  double double_val;
  void* ptr_val;
};

/* TYPE_POINTER: Pointer type to simple_struct */
typedef simple_struct* struct_ptr GTY((tag("STRUCT_PTR")));

/* TYPE_ARRAY: Struct containing an array */
struct GTY(()) array_container {
  int numbers[20];
  char name[50];
};

/* TYPE_SCALAR: Scalar typedef with length attribute */
typedef unsigned long my_scalar GTY((length));

/* TYPE_STRING: String pointer type */
typedef const char* my_string GTY((length));

/* Nested callback structure - callback inside a struct */
struct GTY(()) callback_container {
  simple_callback_fn handler;          /* TYPE_CALLBACK inside struct */
  process_data_fn processor;           /* Another callback */
  struct_ptr next;                     /* TYPE_POINTER */
  int id;
};

/* Union containing a callback */
union GTY(()) callback_union {
  simple_callback_fn callback;         /* TYPE_CALLBACK inside union */
  int callback_id;
  void* user_data;
};

/* Array of callbacks */
struct GTY(()) callback_array {
  simple_callback_fn handlers[5];      /* Array of TYPE_CALLBACK */
  int count;
};

/* Complex nested structure with multiple callback types */
struct GTY(()) complex_structure {
  callback_container container;        /* TYPE_STRUCT containing callbacks */
  callback_union union_member;         /* TYPE_UNION containing callback */
  callback_array array_member;         /* TYPE_STRUCT with callback array */
  my_string description;               /* TYPE_STRING */
  my_scalar length;                    /* TYPE_SCALAR */
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY(()) lang_specific_struct {
  int lang_specific_field;
  /* This would normally have language-specific fields */
};

#endif /* CALLBACK_TYPES_H */
