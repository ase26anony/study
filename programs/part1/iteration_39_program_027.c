/* Test header with diverse GTY-annotated types for gengtype coverage */
#ifndef MYTEST_H
#define MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* Forward declarations for struct types */
struct my_base_struct;
union my_base_union;

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_POINTER: Various pointer types */
struct my_base_struct * GTY(()) my_struct_pointer;
union my_base_union * GTY(()) my_union_pointer;
my_scalar_t * GTY(()) my_scalar_pointer;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_base_struct * GTY(()) my_struct_array[5];

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_base_struct {
  int field1;
  my_scalar_t field2;
  struct my_base_struct * GTY((skip)) next;  /* Skip this field for GC */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  /* User must provide marking functions */
};

/* TYPE_UNION: Union type */
union GTY(()) my_base_union {
  int int_val;
  char * GTY((tag("0"))) str_val;
  struct my_base_struct * GTY((tag("1"))) struct_val;
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
};
#endif

/* Nested structure for additional coverage */
struct GTY(()) outer_struct {
  struct GTY((desc("%1.type"))) inner_struct {
    int type;
    union {
      int ival;
      double dval;
    } GTY((desc("%0.type"))) value;
  } inner;
  
  /* Array of pointers */
  struct inner_struct * GTY((length("count"))) items;
  unsigned int count;
};

/* Variable-length array using length attribute */
struct GTY(()) varray_struct {
  int size;
  int GTY((length("%h.size"))) data[1];
};

/* Chain of structures for linked list testing */
struct GTY(()) chain_node {
  int id;
  struct chain_node * GTY((skip)) next;  /* Skip to avoid cycles */
  struct chain_node * GTY((skip)) prev;
};

/* Test structure with callback field */
struct GTY(()) callback_container {
  const char *name;
  my_callback_fn GTY((skip)) handler;  /* Skip function pointer */
  void * GTY((skip)) user_data;
};

#endif /* MYTEST_H */
