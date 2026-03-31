/* Test header for gengtype coverage - contains all type categories */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"

/* Forward declarations for struct/union types */
struct my_forward_struct;
union my_forward_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  struct my_forward_struct *field2;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*marker)(void *);
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY(()) global_union_ptr;
extern my_scalar_t * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY(()) int_array[10];
extern struct my_test_struct * GTY(()) struct_ptr_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char *);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
};
#endif

/* TYPE_UNDEFINED: Forward declared struct (will be TYPE_UNDEFINED initially) */
struct my_forward_struct {
  int value;
};

/* Another undefined type reference */
extern struct undefined_type * GTY(()) undefined_ptr;

/* Complex nested example to ensure multiple passes */
struct GTY(()) container_struct {
  my_scalar_t id;
  struct my_test_struct * GTY((skip)) nested_struct;
  union my_test_union data;
  int GTY((length ("%h.length_field"))) variable_array[1];
};

/* Union with pointer alternative */
union GTY(()) ptr_union {
  struct my_test_struct * GTY((tag ("0"))) struct_ptr;
  union my_test_union * GTY((tag ("1"))) union_ptr;
  void * GTY((tag ("2"))) generic_ptr;
};

#endif /* MYTEST_H */
