/* Test header with diverse GTY-annotated types for gengtype coverage */
#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;
};

/* TYPE_USER_STRUCT: User-defined struct with tag */
struct GTY((user)) my_user_struct {
  int data;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  void * GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_ptr;
union my_test_union * GTY(()) my_union_ptr;
my_scalar_t * GTY(()) my_scalar_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
struct my_test_struct GTY(()) my_struct_array[5];
const char * GTY(()) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((desc("%0.lang_code"))) lang_specific_struct {
  int lang_code;
  void * GTY((skip)) lang_data;
};

/* Nested structures for complex testing */
struct GTY(()) outer_struct {
  struct my_test_struct inner;
  struct GTY((chain_next ("%h.next"))) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
  } *node_list;
  
  /* Array of pointers */
  struct linked_node * GTY(()) node_array[8];
  
  /* Union within struct */
  union {
    int tag;
    double value;
  } GTY(()) variant;
};

/* Forward declaration with GTY */
struct GTY(()) forward_declared;

/* Complete the forward declaration */
struct GTY(()) forward_declared {
  int id;
  struct forward_declared * GTY((skip)) next;
};

/* Template-like structure with conditional fields */
#ifdef ENABLE_FEATURE
struct GTY(()) conditional_struct {
  int enabled_field;
};
#endif

/* Multiple inheritance-like structure using unions */
struct GTY(()) base1 {
  int base1_data;
};

struct GTY(()) base2 {
  double base2_data;
};

struct GTY(()) derived {
  struct base1 b1;
  struct base2 b2;
  int derived_data;
};

#endif /* GCC_MYTEST_H */
