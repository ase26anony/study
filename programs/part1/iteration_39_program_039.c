/* Test header for gengtype coverage - covers all type categories */
#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  void * GTY((skip)) ptr_val;
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

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef GENERATOR_FILE
struct GTY(()) my_lang_struct {
  int lang_specific_field;
  struct my_test_struct * GTY((skip)) associated_struct;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) description;  /* TYPE_STRING */
  struct my_test_struct data;        /* TYPE_STRUCT */
  union my_test_union variant;       /* TYPE_UNION */
  int GTY(()) counts[20];            /* TYPE_ARRAY */
  struct complex_container * GTY((skip)) next; /* TYPE_POINTER */
  my_callback_fn GTY(()) handler;    /* TYPE_CALLBACK */
};

/* Forward declaration for pointer types */
struct forward_declared;
struct forward_declared * GTY(()) forward_ptr;

/* Another struct using the forward declared type */
struct GTY(()) uses_forward {
  struct forward_declared * GTY((skip)) fwd_ptr;
  int id;
};

#endif /* GCC_MYTEST_H */
