/* gtype-test.cc - Comprehensive type coverage test for gengtype */
/* This file should be placed in the gcc/ directory and built as part of GCC */

/* Boilerplate GCC headers */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Forward declarations for undefined types */
struct OpaqueStruct;          /* TYPE_UNDEFINED */
union OpaqueUnion;            /* TYPE_UNDEFINED */

/* Basic scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int;
typedef float GTY(()) scalar_float;
typedef double GTY(()) scalar_double;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;

/* String type - TYPE_STRING */
typedef const char * GTY(()) string_type;

/* Simple struct - TYPE_STRUCT */
struct GTY(()) SimpleStruct {
  int x;
  float y;
};

/* User struct via typedef - TYPE_USER_STRUCT */
typedef struct GTY(()) {
  int id;
  char name[32];
} UserStruct;

/* Another struct with GTY markers */
struct GTY(()) ComplexStruct {
  int *GTY((skip)) raw_ptr;           /* Pointer without GC tracking */
  struct SimpleStruct *GTY(()) gty_ptr; /* GC-tracked pointer */
};

/* Union type - TYPE_UNION */
union GTY(()) TestUnion {
  int as_int;
  float as_float;
  struct SimpleStruct *GTY(()) as_struct;
};

/* Pointer types - TYPE_POINTER */
typedef int * GTY(()) int_ptr;
typedef struct SimpleStruct * GTY(()) struct_ptr;
typedef union TestUnion * GTY(()) union_ptr;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct SimpleStruct GTY(()) struct_array[5];
extern int GTY(()) incomplete_array[];  /* Incomplete array */

/* Complex nested array */
struct GTY(()) ArrayContainer {
  int GTY(()) matrix[3][3];
  struct SimpleStruct *GTY(()) ptr_array[4];
};

/* Callback/function types - TYPE_CALLBACK */
typedef int (*GTY(()) simple_callback)(int, float);
typedef void (*GTY(()) complex_callback)(struct SimpleStruct *, union TestUnion *);

/* Function pointer with complex return type */
typedef struct SimpleStruct * (*GTY(())) callback_returning_struct(void);

/* Struct containing callback */
struct GTY(()) CallbackHolder {
  simple_callback cb1;
  complex_callback cb2;
  callback_returning_struct cb3;
};

/* Language-specific struct - TYPE_LANG_STRUCT */
/* Use GCC attributes to potentially trigger language-specific handling */
struct GTY(()) LangStruct __attribute__((aligned(16))) {
  int data;
  void *GTY(()) ptr;
};

/* Another with transaction_safe attribute */
struct GTY(()) TransactionStruct __attribute__((transaction_safe)) {
  int value;
  struct LangStruct *GTY(()) lang_ptr;
};

/* Complex nested type combining multiple categories */
struct GTY(()) SuperNested {
  /* Scalar members */
  int id;
  float score;
  
  /* String member */
  const char *GTY(()) description;
  
  /* Struct member */
  struct SimpleStruct embedded;
  
  /* Union member */
  union TestUnion variant;
  
  /* Pointer members */
  struct ComplexStruct *GTY(()) complex_ptr;
  int *GTY(()) int_ptr;
  
  /* Array members */
  UserStruct GTY(()) users[8];
  callback_returning_struct GTY(()) callbacks[3];
  
  /* Nested array of pointers */
  struct LangStruct *GTY(()) lang_ptrs[2][2];
  
  /* Callback member */
  complex_callback processor;
};

/* Global variables to ensure types are used */
struct SimpleStruct GTY(()) global_struct = {1, 2.0};
UserStruct GTY(()) global_user_struct = {42, "test"};
union TestUnion GTY(()) global_union;
int_array GTY(()) global_array = {0};
struct LangStruct GTY(()) global_lang_struct = {0, NULL};
struct TransactionStruct GTY(()) global_trans_struct = {0, NULL};

/* Function using the complex types */
void GTY(()) gt_test_function(void)
{
  /* Use the global variables */
  global_struct.x = 10;
  global_user_struct.id = 100;
  global_union.as_int = 42;
  
  /* Create local instances */
  struct ComplexStruct local_complex;
  struct SuperNested local_nested;
  struct CallbackHolder local_holder;
  
  /* Initialize some members */
  local_nested.id = 1;
  local_nested.description = "Nested structure";
  local_nested.processor = NULL;
  
  /* Use array types */
  for (int i = 0; i < 10; i++) {
    global_array[i] = i * 2;
  }
  
  /* Use pointer types */
  int_ptr dynamic_int = NULL;
  struct_ptr dynamic_struct = &global_struct;
  
  (void)local_complex;  /* Prevent unused warning */
  (void)local_nested;
  (void)local_holder;
  (void)dynamic_int;
  (void)dynamic_struct;
}

/* Additional complex type definitions in a header-like section */
#ifndef GTYPE_TEST_HEADER
#define GTYPE_TEST_HEADER

/* Another undefined type */
struct AnotherOpaque;

/* Typedef for function pointer returning pointer to array */
typedef int (*GTY(()))(*callback_returning_array_ptr(void))[10];

/* Union containing struct and callback */
union GTY(()) MixedUnion {
  struct {
    int type;
    void *GTY(()) data;
  } GTY(()) header;
  simple_callback handler;
};

#endif /* GTYPE_TEST_HEADER */

/* Main function to make this a complete source file */
int main(int argc, char **argv)
{
  gt_test_function();
  return 0;
}
