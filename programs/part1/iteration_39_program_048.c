/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_H
#define MYTEST_H

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
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: Forward declared struct that will be defined elsewhere */
struct my_user_struct;
typedef struct my_user_struct * GTY(()) my_user_struct_ptr;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  const char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Pointer type with GTY annotation */
typedef struct my_test_struct * GTY(()) my_struct_ptr;

/* TYPE_ARRAY: Array type with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
/* This mimics language-specific structures in GCC */
struct GTY(()) my_lang_struct {
  int lang_specific_field;
  struct my_test_struct * GTY((tag("0"))) data;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr; /* TYPE_POINTER (to TYPE_STRUCT) */
  union my_test_union union_field;   /* TYPE_UNION */
  my_callback_fn callback;           /* TYPE_CALLBACK */
  
  /* Variable length array */
  int GTY((length("array_len"))) *var_array;
  int array_len;
  
  /* Nested struct */
  struct GTY(()) nested {
    int x;
    int y;
  } nested_field;
};

/* Another struct using the callback type */
struct GTY(()) callback_container {
  const char * GTY((skip)) name;
  my_callback_fn handlers[4];  /* Array of callbacks */
};

/* Union containing pointers of different types */
union GTY(()) pointer_union {
  struct my_test_struct * GTY((tag("1"))) struct_ptr;
  union my_test_union * GTY((tag("2"))) union_ptr;
  void * GTY((tag("0"))) generic_ptr;
};

/* Mark the end of our test types */
extern int GTY(()) test_types_defined;

#endif /* MYTEST_H */
