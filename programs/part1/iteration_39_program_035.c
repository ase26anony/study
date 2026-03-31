/* Test header with GTY annotations for gengtype coverage testing.
   This file contains examples of all type categories that gengtype
   needs to classify in its statistics collection function. */

#ifndef GCC_MYTEST_GTY_H
#define GCC_MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Plain struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
};

/* TYPE_USER_STRUCT: Struct typedef with GTY annotation */
typedef struct GTY(()) {
  int data;
  void *extra;
} my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int as_int;
  char *as_ptr;
  double as_double;
};

/* TYPE_POINTER: Pointer to struct with GTY annotation */
struct my_test_struct * GTY((skip)) my_struct_pointer;

/* TYPE_ARRAY: Array with GTY annotation */
extern int GTY((length("my_array_length"))) my_test_array[10];

/* Variable for array length callback */
extern size_t my_array_length;

/* TYPE_CALLBACK: Function pointer with GTY annotation */
typedef void (*GTY(()) my_callback_fn)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
};
#endif

/* Complex nested example combining multiple types */
struct GTY(()) complex_container {
  /* Scalar field */
  my_scalar_t id;
  
  /* String field */
  const char * GTY((tag("0"))) name;
  
  /* Pointer field */
  struct my_test_struct * GTY((skip)) data_ptr;
  
  /* Array field */
  int GTY((length("count"))) items[5];
  
  /* Union field */
  union my_test_union GTY((desc("type"))) value;
  
  /* Callback field */
  my_callback_fn GTY((skip)) handler;
  
  /* Nested struct */
  struct {
    int x;
    int y;
  } GTY(()) position;
  
  int count;
  int type;
};

/* Another example with chain of pointers */
struct GTY(()) linked_node {
  int value;
  struct linked_node * GTY((skip)) next;
  struct linked_node * GTY((skip)) prev;
};

/* Test variable declarations using our types */
extern struct complex_container GTY(()) global_container;
extern my_user_struct_t GTY(()) user_struct_instance;
extern union my_test_union GTY(()) union_instance;

/* Function declarations that might use these types */
extern void init_test_types(void);
extern void cleanup_test_types(void);

#endif /* GCC_MYTEST_GTY_H */
