/* Test header with GTY annotations for gengtype coverage testing */
/* This file should be placed in the gcc/ directory of the GCC source tree */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  void * GTY((skip)) opaque_data;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC marking */
struct GTY((user)) my_user_struct {
  int data;
  /* User-defined marking function would be declared here */
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Pointer type with GTY annotation */
typedef struct my_test_struct * GTY(()) my_struct_ptr;

/* TYPE_ARRAY: Array type with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%1.type"), tag("MY_LANG_TYPE"))) my_lang_struct {
  enum my_lang_type_enum type;
  union {
    int int_val;
    double float_val;
    struct my_test_struct * GTY((skip)) struct_ptr;
  } GTY((desc("$1.type"))) u;
};

/* Complex nested example to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  const char * GTY(()) name;          /* TYPE_STRING */
  struct my_test_struct nested;       /* TYPE_STRUCT */
  union my_test_union data_union;     /* TYPE_UNION */
  my_struct_ptr next;                 /* TYPE_POINTER */
  int GTY((length("array_len"))) dynamic_array[]; /* TYPE_ARRAY */
  size_t array_len;
  my_callback_fn callback;            /* TYPE_CALLBACK */
};

/* Forward declaration for pointer chain */
struct GTY(()) linked_node;
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip)) next;
};

/* Array of pointers */
typedef struct my_test_struct * GTY(()) struct_ptr_array[10];

/* Union containing pointer */
union GTY(()) ptr_union {
  int * GTY((skip)) int_ptr;
  struct my_test_struct * GTY((skip)) struct_ptr;
};

/* Callback with context */
typedef void (*GTY(()) callback_with_context)(void * GTY((skip)) context, int value);

/* Marked with chain_next for linked list testing */
struct GTY((chain_next("%h.next"))) chainable_struct {
  int id;
  struct chainable_struct *next;
};

#endif /* MYTEST_GTY_H */
