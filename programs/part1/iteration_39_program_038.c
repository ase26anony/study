/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for testing */
struct my_base_struct;
union my_base_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* Skip this field for GC */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  void *data;
  size_t length;
  
  /* User-defined marking function */
  void GTY((user)) mark() { }
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  char * GTY((tag("0"))) string_val;  /* Tagged pointer for union discrimination */
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern my_scalar_t * GTY((length("my_array_length"))) dynamic_array_ptr;

/* TYPE_ARRAY: Fixed-size array with GTY annotation */
extern int GTY(()) my_fixed_array[10];

/* Variable-length array in a struct */
struct GTY(()) array_container {
  int count;
  int GTY((length("%0.count"))) elements[1];  /* Variable length array */
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback_fn)(int, const char*);
extern test_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_field;
  tree GTY((tag("0"))) lang_tree;  /* tree is a GCC internal type */
};
#endif

/* Nested structures to test complex type graphs */
struct GTY(()) outer_struct {
  struct my_test_struct * GTY(()) inner;
  union my_test_union data;
  int GTY(()) scalar_field;
  
  /* Callback field */
  test_callback_fn GTY(()) callback;
  
  /* Array of pointers */
  struct my_test_struct * GTY((length("5"))) ptr_array[5];
};

/* Chain of pointers for testing pointer following */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY(()) next;
  struct linked_node * GTY((skip)) prev;  /* Skip reverse pointer to avoid cycles */
};

/* Union containing pointers */
union GTY(()) pointer_union {
  struct my_test_struct * GTY((tag("0"))) struct_ptr;
  struct linked_node * GTY((tag("1"))) node_ptr;
  void * GTY((tag("2"))) generic_ptr;
};

/* Test structure with all type categories */
struct GTY(()) comprehensive_test {
  /* Scalar */
  my_scalar_t scalar;
  
  /* String */
  const char * GTY(()) description;
  
  /* Struct */
  struct my_test_struct embedded;
  
  /* Pointer */
  struct outer_struct * GTY(()) outer;
  
  /* Array */
  int GTY(()) numbers[3];
  
  /* Union */
  union my_test_union data_union;
  
  /* Callback */
  test_callback_fn GTY(()) handler;
  
  /* Chain */
  struct linked_node * GTY(()) chain_head;
};

/* Global instances for testing */
extern struct comprehensive_test GTY(()) global_test_instance;
extern struct linked_node GTY(()) node_pool[20];

#endif /* GCC_MYTEST_H */
