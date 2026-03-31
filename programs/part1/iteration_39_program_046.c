/* Test header for gengtype coverage - covers all type categories in statistics collection */

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

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(void*);
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Pointer to struct with GTY annotation */
typedef struct my_test_struct * GTY(()) my_struct_ptr;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[];

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.type"), tag("lang_type"))) lang_type_struct {
  int type;
  union GTY((desc("%1.type"))) {
    int int_val;
    double double_val;
  } GTY((tag("0"))) u;
};

/* Complex nested example to ensure deep processing */
struct GTY(()) complex_container {
  /* Nested pointer */
  struct my_test_struct * GTY(()) nested_ptr;
  
  /* Array of pointers */
  struct my_test_struct * GTY((length("array_len"))) ptr_array[];
  
  /* Callback field */
  my_callback_fn callback;
  
  /* Union field */
  union my_test_union data_union;
  
  /* Scalar field */
  my_scalar_t scalar_field;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct GTY(()) forward_declared {
  int value;
  struct forward_declared * GTY((skip)) next;
};

/* Variable declarations using our types */
extern struct my_test_struct GTY(()) global_struct;
extern union my_test_union GTY(()) global_union;
extern my_struct_ptr GTY(()) global_ptr_array[5];
extern my_callback_fn GTY(()) callbacks[3];

#endif /* MYTEST_H */
