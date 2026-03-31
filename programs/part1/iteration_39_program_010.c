/* Test header for gengtype coverage - covers all type categories in statistics */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_type field2;
  const char * GTY((skip)) name;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_base_struct GTY(()) my_user_struct;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_base_struct * GTY((tag("0"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct my_base_struct * GTY(()) global_struct_ptr;
my_scalar_type * GTY(()) scalar_ptr;
union my_test_union * GTY(()) union_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
struct my_base_struct GTY(()) struct_array[5];
my_scalar_type GTY(()) scalar_array[20];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback)(int, struct my_base_struct *);
extern test_callback GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
  struct my_base_struct * GTY(()) base;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) container_struct {
  /* Multiple pointer types */
  struct my_base_struct ** GTY(()) ptr_to_ptr;
  
  /* Array of pointers */
  union my_test_union * GTY(()) union_array[8];
  
  /* Callback in struct */
  test_callback GTY(()) handler;
  
  /* String array */
  const char * GTY(()) string_array[4];
  
  /* Self-referential pointer */
  struct container_struct * GTY(()) next;
};

/* Variable length array with length specifier */
struct GTY(()) var_len_struct {
  int count;
  int GTY((length("%0.count"))) data[1];
};

/* Union with variable tagging */
union GTY((desc("%0.type"))) tagged_union {
  int type;
  struct my_base_struct GTY((tag("1"))) as_struct;
  union my_test_union GTY((tag("2"))) as_union;
};

/* For TYPE_UNDEFINED testing - forward declared struct used in pointer */
struct undefined_struct;
struct undefined_struct * GTY(()) undefined_ptr;

#endif /* MYTEST_GTY_H */
