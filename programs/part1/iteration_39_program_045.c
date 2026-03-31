/* Test header for gengtype coverage - contains all type categories */
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
  int GTY((skip)) int_field;
  my_scalar_t scalar_field;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types with GTY annotations */
extern struct my_test_struct * GTY(()) my_struct_pointer;
extern my_scalar_t * GTY((length("0"))) my_scalar_array_pointer;
extern union my_test_union * GTY(()) my_union_pointer;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY((length("10"))) my_fixed_array[10];

/* TYPE_CALLBACK: Function pointer (callback) with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  const char * GTY(()) name;          /* TYPE_STRING */
  struct my_test_struct * GTY(()) next; /* TYPE_POINTER */
  int GTY((length("5"))) scores[5];   /* TYPE_ARRAY */
  union my_test_union data;           /* TYPE_UNION */
  void (*GTY(()) handler)(void);      /* TYPE_CALLBACK */
};

/* Forward declaration for pointer chain */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip)) next;
};

/* Template for array of pointers */
typedef struct my_test_struct * GTY((length("0"))) struct_ptr_array[];

#endif /* MYTEST_H */
