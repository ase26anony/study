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

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) optional_name;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  void * GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
my_scalar_t * GTY(()) my_scalar_pointer;
union my_test_union * GTY(()) my_union_pointer;

/* TYPE_ARRAY: Array types with GTY annotations */
int GTY(()) my_int_array[10];
struct my_test_struct GTY(()) my_struct_array[5];
const char * GTY(()) string_array[3];

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
typedef int (*GTY(()) another_callback)(struct my_test_struct *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_identifier {
  const char * GTY((skip)) id;
  int token;
};
#endif

/* Complex nested example to ensure deep processing */
struct GTY(()) container_struct {
  /* Nested struct */
  struct GTY(()) nested {
    int x;
    int y;
  } point;
  
  /* Pointer array */
  struct my_test_struct * GTY((length("count"))) items;
  int count;
  
  /* Callback */
  my_callback_fn GTY((skip)) handler;
  
  /* Union field */
  union my_test_union data;
  
  /* String */
  const char * GTY(()) name;
};

/* Variable-length array using GTY((length)) */
struct GTY(()) varray_struct {
  int size;
  int GTY((length("size"))) elements[1];
};

/* For TYPE_UNDEFINED coverage - forward declaration */
struct GTY(()) undefined_struct;
typedef struct undefined_struct * GTY(()) undefined_ptr;

#endif /* MYTEST_GTY_H */
