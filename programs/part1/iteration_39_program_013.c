/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
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
extern int GTY((length("my_array_length"))) my_test_array[];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((desc("%0.lang_code"))) my_lang_struct {
  int lang_code;
  union my_test_union data;
};

/* Complex nested example covering multiple categories */
struct GTY(()) complex_container {
  /* Scalar field */
  my_scalar_t id;
  
  /* String field */
  const char * GTY(()) name;
  
  /* Pointer field */
  struct my_test_struct * GTY(()) data_ptr;
  
  /* Array field */
  int GTY((length("array_len"))) *dynamic_array;
  size_t array_len;
  
  /* Union field */
  union my_test_union value;
  
  /* Callback field */
  my_callback_fn callback;
  
  /* Nested struct */
  struct GTY(()) nested {
    int x;
    int y;
  } point;
};

/* Forward declaration for pointer type */
struct forward_declared;
struct forward_declared * GTY(()) forward_ptr;

/* Another scalar type with different GTY options */
typedef unsigned long GTY((skip)) my_skip_scalar;

/* Array of pointers */
struct my_test_struct * GTY((length("ptr_array_len"))) *ptr_array;
size_t ptr_array_len;

/* Union with nested struct */
union GTY(()) nested_union {
  struct {
    int a;
    int b;
  } s;
  long long l;
};

#endif /* MYTEST_H */
