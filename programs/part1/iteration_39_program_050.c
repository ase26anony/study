/* Test header for gengtype coverage - contains all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_test_struct;
union my_test_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_type field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  void *data;
  size_t length;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char *string_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) global_struct_ptr;
union my_test_union * GTY(()) global_union_ptr;
my_scalar_type * GTY(()) scalar_ptr_array[10];

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[100];
struct my_test_struct GTY(()) struct_array[50];
char * GTY((length("strlen(%h) + 1"))) string_array[20];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback)(int, const char*);
extern test_callback GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_field;
  void *lang_data;
};
#endif

/* Nested and complex types for comprehensive testing */
struct GTY(()) container_struct {
  /* Multiple pointer types */
  struct my_test_struct * GTY(()) first;
  union my_test_union * GTY(()) second;
  
  /* Array of pointers */
  struct my_test_struct * GTY(()) children[10];
  
  /* Callback field */
  test_callback GTY(()) handler;
  
  /* String field */
  const char * GTY(()) name;
  
  /* Scalar field */
  int GTY(()) count;
  
  /* Union field */
  union my_test_union GTY(()) data;
};

/* Template-like structure with conditional fields */
struct GTY(()) variable_struct {
  enum { MODE_A, MODE_B, MODE_C } mode;
  
  /* Conditional pointer based on mode */
  union {
    struct my_test_struct * GTY((tag("MODE_A"))) ptr_a;
    union my_test_union * GTY((tag("MODE_B"))) ptr_b;
    char * GTY((tag("MODE_C"))) ptr_c;
  } u;
};

/* Chain structure for linked list testing */
struct GTY(()) chain_element {
  int id;
  const char * GTY(()) label;
  struct chain_element * GTY(()) next;
  struct chain_element * GTY(()) prev;
};

/* Test structure with skip annotation */
struct GTY((skip)) skipped_struct {
  void *opaque_data;  /* Won't be traced by GC */
  int visible_field;
};

#endif /* MYTEST_GTY_H */
