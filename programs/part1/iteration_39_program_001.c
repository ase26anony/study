/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) my_user_struct {
  int data;
  void (* GTY((skip)) cleanup)(void);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((skip)) string_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_ptr;
union my_test_union * GTY(()) my_union_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
  tree chain;
};
#endif

/* Nested/complex types for thorough testing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
  int GTY((length("array_len"))) *variable_array; /* TYPE_ARRAY with length */
  int array_len;
  my_callback_fn callback;           /* TYPE_CALLBACK */
};

/* Template-like macro usage (common in GCC) */
#define DEF_GTY_STRUCT(name) \
  struct GTY(()) name { \
    int id; \
    struct name * GTY((skip)) next; \
  }

DEF_GTY_STRUCT(linked_list_node);

/* Another union with struct members */
union GTY(()) complex_union {
  struct {
    int type;
    void * GTY((skip)) data;
  } s;
  long long int combined;
};

#endif /* MYTEST_GTY_H */
