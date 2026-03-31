/* Test header for gengtype coverage - contains various GTY-annotated types
   to trigger all cases in the type statistics collection function. */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct types */
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

/* TYPE_USER_STRUCT: Struct with user-defined GC marking */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(void*);
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
extern union my_test_union * GTY((skip)) union_ptr_skip;
extern my_scalar_t * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY(()) fixed_array[100];
extern struct my_test_struct * GTY((length("len"))) variable_array[];
extern const char * GTY(()) string_array[10];

/* TYPE_CALLBACK: Function pointer type with GTY annotation */
typedef void (*GTY(()) callback_func_t)(int, const char*);
extern callback_func_t GTY(()) registered_callback;

/* TYPE_LANG_STRUCT: Language-specific struct (simulated) */
struct GTY((desc("%0.lang_code"))) lang_specific_struct {
  int lang_code;
  void *language_data;
};

/* Complex nested type to ensure thorough processing */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) name;         /* TYPE_STRING */
  struct my_test_struct * GTY(()) data; /* TYPE_POINTER */
  union my_test_union GTY(()) variant; /* TYPE_UNION */
  int GTY(()) scores[5];             /* TYPE_ARRAY */
  callback_func_t GTY(()) handler;   /* TYPE_CALLBACK */
  struct lang_specific_struct * GTY(()) lang_data; /* TYPE_LANG_STRUCT */
};

/* Template-like macro to generate multiple instances */
#define DECLARE_TEST_TYPE(name, type) \
  extern type GTY(()) test_##name

DECLARE_TEST_TYPE(ptr1, struct my_test_struct*);
DECLARE_TEST_TYPE(ptr2, union my_test_union*);
DECLARE_TEST_TYPE(arr1, int[20]);
DECLARE_TEST_TYPE(cb1, callback_func_t);

/* Inline function using GTY types (for parsing but not for GC) */
static inline void GTY((no)) process_data(struct my_test_struct * GTY(()) data) {
  if (data && data->handler) {
    data->handler(data->field1, "processed");
  }
}

#endif /* MYTEST_GTY_H */
