/* Test header for covering gengtype.cc statistics collection */
/* This file contains examples of all GTY-annotated type categories */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Simple scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC marking */
struct GTY((user)) my_user_struct {
  void *data;
  size_t length;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  void *ptr_val;
};

/* TYPE_POINTER: Pointer to struct with GTY annotation */
struct my_test_struct * GTY(()) my_struct_pointer;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY(()) my_int_array[10];

/* Array of pointers */
struct my_test_struct * GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* Struct containing a callback */
struct GTY(()) struct_with_callback {
  my_callback_fn callback;
  int id;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_identifier {
  const char *name;
  int value;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_field;
    struct outer_struct * GTY((skip)) parent;
  } *inner;
  
  union GTY(()) inner_union {
    int a;
    float b;
  } u;
  
  my_scalar_t scalar_field;
  my_int_array array_field;  /* Array decay to pointer */
};

/* Variable length array with length parameter */
struct GTY(()) varray_struct {
  int length;
  int GTY((length ("%h.length"))) items[1];
};

/* Chain of structures */
struct GTY(()) chain_node {
  int value;
  struct chain_node * GTY((chain_next ("%h.next"))) next;
};

/* Use GTY((skip)) for pointers that shouldn't be traced */
struct GTY(()) skip_example {
  void * GTY((skip)) opaque_pointer;
  struct skip_example * GTY(()) traceable_pointer;
};

/* Template-like structure for C++ compatibility */
#ifdef __cplusplus
template<typename T>
struct GTY(()) template_wrapper {
  T value;
  template_wrapper<T> *next;
};
#endif

/* Enumeration type (treated as scalar) */
typedef enum GTY(()) {
  STATE_A,
  STATE_B,
  STATE_C
} my_state_t;

/* Forward declaration with GTY */
struct GTY(()) forward_declared_struct;

/* Complete the forward declaration */
struct GTY(()) forward_declared_struct {
  int complete_field;
  struct forward_declared_struct *next;
};

/* Multiple inheritance simulation for C */
struct GTY(()) base_struct {
  int base_field;
};

struct GTY(()) derived_struct {
  struct base_struct base;
  int derived_field;
};

#endif /* MYTEST_GTY_H */
