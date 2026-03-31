/* Test header with diverse GTY-annotated types for gengtype coverage */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer to struct with GTY annotation */
struct my_test_struct * GTY((skip)) my_struct_pointer;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[10];
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, struct my_test_struct *);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  struct my_test_struct *base;
  int lang_specific;
};
#endif

/* Complex nested example to ensure deep processing */
struct GTY(()) container_struct {
  /* Multiple pointer types */
  struct my_test_struct ** GTY((length("ptr_count"))) ptr_array;
  int ptr_count;
  
  /* Union field */
  union my_test_union data;
  
  /* Callback field */
  my_callback_fn callback;
  
  /* String field */
  const char * GTY(()) name;
  
  /* Scalar field */
  my_scalar_t id;
};

/* Forward declaration for mutual reference */
struct GTY(()) forward_decl_struct;
struct GTY(()) another_struct {
  struct forward_decl_struct * GTY(()) next;
  int value;
};

struct GTY(()) forward_decl_struct {
  struct another_struct * GTY(()) partner;
  const char * GTY(()) tag;
};

/* Variable declarations using our types */
extern struct container_struct GTY(()) global_container;
extern union my_test_union GTY(()) global_union_array[5];

#endif /* MYTEST_H */
