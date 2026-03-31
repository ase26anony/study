/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  void *data;
  size_t length;
  
  /* User-defined marking function */
  void GTY((user)) mark(void *ptr);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
union my_test_union * GTY((tag("0"))) my_union_pointer;

/* TYPE_ARRAY: Array types */
int GTY(()) my_int_array[10];
struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY((desc("%1.type"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
  enum tree_code type;
  struct lang_struct *next;
  struct lang_struct *prev;
  void *data;
};
#endif

/* Nested structures to ensure complex type graph */
struct GTY(()) outer_struct {
  struct GTY((skip)) inner_struct {
    int inner_field;
    struct outer_struct *parent;
  } inner;
  
  struct GTY(()) sibling {
    int count;
    struct sibling *next;
  } *sibling_list;
};

/* Variable length array with length specifier */
struct GTY(()) var_len_struct {
  int count;
  int GTY((length("%0.count"))) items[1];
};

/* Pointer with special handling */
typedef struct GTY((maybe_undef)) maybe_undefined *maybe_undef_ptr;

#endif /* GCC_MYTEST_H */
