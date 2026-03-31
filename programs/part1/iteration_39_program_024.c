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
  const char * GTY((skip)) name;  /* skip annotation for variety */
};

/* TYPE_USER_STRUCT: Struct with user-defined GC markers */
struct GTY((user)) my_user_struct {
  int data;
  void (*GTY((skip)) mark_func)(void *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  void * GTY((skip)) ptr_val;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
my_scalar_t * GTY(()) my_scalar_pointer;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char *);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY((desc("%0.type"), tag("MY_LANG_TYPE"))) my_lang_struct {
  enum my_lang_type_enum type;
  union {
    int int_val;
    struct my_test_struct * GTY((tag("0"))) struct_ptr;
  } GTY((desc("%0.type"))) u;
};
#endif

/* Forward declaration for pointer chain */
struct GTY(()) forward_decl_struct;

/* Complex nested type to ensure deep processing */
struct GTY(()) container_struct {
  struct my_test_struct base;
  struct forward_decl_struct * GTY((skip)) next;
  union my_test_union data;
  my_callback_fn callback;
  int GTY(()) dynamic_array[0];  /* Zero-length array */
};

/* Another pointer type using forward declaration */
struct forward_decl_struct * GTY(()) global_forward_ptr;

/* Parameterized pointer type */
typedef struct my_test_struct * GTY((length("len"))) my_length_ptr;
extern int len;

/* TYPE_UNDEFINED: This should be caught by default case */
/* Not explicitly created - will be handled by any unclassified types */

#endif /* GCC_MYTEST_H */
