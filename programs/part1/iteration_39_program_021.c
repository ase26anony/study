/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Plain struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
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
extern int GTY((length("my_array_length"))) my_test_array[];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), tag("lang_type"))) my_lang_struct {
  enum tree_code type;
  union my_test_union value;
};

/* Complex nested example to ensure deep processing */
struct GTY(()) complex_container {
  /* Nested pointer */
  struct my_test_struct **GTY((skip)) ptr_array;
  
  /* Array of pointers */
  struct my_test_struct *GTY((length("container_count"))) item_list[];
  
  /* Callback field */
  my_callback_fn GTY((skip)) callback;
  
  /* Union field */
  union my_test_union data;
  
  int container_count;
};

/* Forward declaration for pointer types */
struct forward_declared;
struct forward_declared * GTY(()) forward_ptr;

/* Another scalar type with different annotation */
typedef unsigned long GTY((skip)) my_skip_scalar;

#endif /* MYTEST_GTY_H */
