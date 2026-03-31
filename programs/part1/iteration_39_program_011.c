/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"

/* Forward declarations for pointer types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int GTY(()) field2;
  const char * GTY(()) name;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(struct my_user_struct *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((skip)) union_ptr_skip;
extern my_scalar_t * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct * GTY(()) struct_ptr_array[5];
extern const char * GTY((length("strlen(%h) + 1"))) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback_fn)(int, struct my_test_struct *);
extern test_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_data;
  void * GTY((skip)) opaque_handle;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) outer_container {
  struct my_test_struct * GTY(()) inner_struct;
  union my_test_union GTY(()) data_union;
  int GTY(()) count;
  struct outer_container * GTY((skip)) next;
};

/* Variable length array with length specifier */
struct GTY(()) var_len_struct {
  int length;
  int GTY((length("%0.length"))) flexible_array[];
};

/* Chain of structures for traversal testing */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chainable {
  int id;
  struct chainable * GTY((skip)) next;
  struct chainable * GTY((skip)) prev;
};

#endif /* MYTEST_H */
