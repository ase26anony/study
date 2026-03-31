/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_type field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*cleanup)(struct my_user_struct *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char *string_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) global_struct_ptr;
union my_test_union * GTY(()) global_union_ptr;
my_scalar_type * GTY(()) scalar_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func)(int, const char*);
extern callback_func GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_data;
  void *lang_handle;
};
#endif

/* Nested structures to ensure thorough processing */
struct GTY(()) outer_struct {
  struct GTY(()) inner_struct {
    int inner_data;
    struct inner_struct * GTY((skip)) inner_ptr;
  } inner;
  
  union GTY(()) inner_union {
    int a;
    char b[20];
  } u;
  
  my_scalar_type scalar_field;
  int GTY(()) scalar_array[5];
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct forward_declared {
  int data;
  struct forward_declared * GTY((chain_next("%s.next"))) next;
};

/* Variable length array with length specifier */
struct GTY(()) var_len_struct {
  int length;
  int GTY((length("%0.length"))) data[];
};

/* Skip annotation test */
struct GTY((skip)) skipped_struct {
  void *opaque_data;
};

/* Atomic types */
typedef _Atomic int GTY(()) atomic_scalar;

#endif /* MYTEST_GTY_H */
