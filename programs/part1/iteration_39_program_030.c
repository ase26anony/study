/* Test header for gengtype coverage - contains various GTY-annotated types
   to trigger all classification cases in the statistics collection function. */

#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct/union types */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_t field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(struct my_user_struct *);
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char *string_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types with GTY annotations */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern union my_test_union * GTY((skip)) global_union_ptr;
extern my_scalar_t * GTY(()) scalar_ptr_array[10];

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY(()) int_array[50];
extern struct my_test_struct GTY(()) struct_array[5];
extern char * GTY((length("strlen(%h) + 1"))) string_array[20];

/* TYPE_CALLBACK: Function pointer type with GTY annotation */
typedef void (*GTY(()) callback_func)(int, const char *);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) my_lang_struct {
  int lang_specific_data;
  void *lang_handle;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) description;  /* TYPE_STRING */
  struct my_test_struct *first;      /* TYPE_POINTER to TYPE_STRUCT */
  union my_test_union data;          /* TYPE_UNION */
  int GTY(()) scores[100];           /* TYPE_ARRAY */
  callback_func handler;             /* TYPE_CALLBACK */
  
  /* Pointer to array */
  int (* GTY(()) matrix)[10][10];    /* TYPE_POINTER to TYPE_ARRAY */
  
  /* Self-referential pointer */
  struct complex_container *next;    /* TYPE_POINTER */
};

/* Template-like macro to generate multiple instances */
#define DECLARE_TEST_TYPE(name, base) \
  typedef base GTY(()) name##_t; \
  extern name##_t GTY(()) name##_var; \
  extern name##_t * GTY(()) name##_ptr;

/* Generate more scalar types */
DECLARE_TEST_TYPE(test_int, int)
DECLARE_TEST_TYPE(test_long, long)
DECLARE_TEST_TYPE(test_ptr, void *)

/* Enumeration type (treated as scalar for GC purposes) */
typedef enum GTY(()) {
  STATE_A,
  STATE_B,
  STATE_C
} test_state_t;

/* Union containing pointers */
union GTY(()) ptr_union {
  void * GTY((tag("0"))) generic_ptr;
  struct my_test_struct * GTY((tag("1"))) struct_ptr;
  int * GTY((tag("2"))) int_ptr;
};

#endif /* MYTEST_H */
