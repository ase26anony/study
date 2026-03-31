/* Test header for gengtype coverage - contains all type categories */
#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int GTY((skip)) field2;  /* Skip this field for GC */
  const char * GTY((length("strlen(%h.field3) + 1"))) field3; /* String with length */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  void *data;
  
  /* User-defined markers */
  void GTY((mark)) (*marker)(void *);
  void * GTY((skip)) opaque;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  my_scalar_t scalar_val;
  struct my_test_struct * GTY((tag("0"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((chain_next("%h.next"))) union_ptr_chain;
typedef struct my_test_struct * GTY(()) struct_ptr_t;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
extern struct my_test_struct * GTY((length("%h.count"))) ptr_array[];
typedef int GTY(()) matrix_t[5][5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_identifier {
  const char *name;
  unsigned int hash;
};
#endif

/* Nested structures for complex testing */
struct GTY(()) outer_container {
  /* Contains one of each type category */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) string_field; /* TYPE_STRING */
  struct my_test_struct struct_field; /* TYPE_STRUCT */
  union my_test_union union_field;    /* TYPE_UNION */
  struct my_test_struct *ptr_field;   /* TYPE_POINTER */
  int array_field[5];                 /* TYPE_ARRAY */
  void (*GTY(()) callback_field)(void); /* TYPE_CALLBACK */
  
  /* Chain for linked list testing */
  struct outer_container * GTY((chain_next("%h.next"))) next;
};

/* Template-like structure for parameterized types */
struct GTY(()) param_struct {
  enum { TYPE_INT, TYPE_PTR, TYPE_STRUCT } kind;
  
  union {
    int int_val;
    void *ptr_val;
    struct my_test_struct *struct_val;
  } GTY((desc("%0.kind"))) value;
};

/* For testing TYPE_UNDEFINED - forward declared but never defined */
struct GTY(()) undefined_struct;

#endif /* GCC_MYTEST_H */
