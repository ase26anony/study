/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for coverage */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*GTY((skip)) cleanup)(void);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
union my_test_union * GTY((chain_next ("%h.next"))) my_union_pointer;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY((desc ("%h.type"))) my_lang_struct {
  enum tree_code type;
  int lang_specific;
};
#endif

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
  struct GTY((tag ("0"))) inner_struct {
    int inner_field;
  } inner;
  
  union GTY(()) inner_union {
    int a;
    char b;
  } u;
};

/* Variable length array with length annotation */
struct GTY(()) var_len_struct {
  int length;
  int GTY((length ("%h.length"))) data[];
};

/* Pointer with callback for tracing */
struct GTY((for_user)) traced_pointer {
  void * GTY((callback ("my_trace_fn"))) ptr;
};

/* Forward declaration for pointer chain */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip)) next;
};

#endif /* GCC_MYTEST_H */
