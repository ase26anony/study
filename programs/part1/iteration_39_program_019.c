/* Test header for gengtype coverage - covers all type categories */
#ifndef MYTEST_GTY_H
#define MYTEST_GTY_H

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
  const char * GTY((skip)) name;  /* Skip this field for GC */
};

/* TYPE_USER_STRUCT: Forward declared struct */
struct my_user_struct;
typedef struct my_user_struct * GTY(()) my_user_struct_ptr;

struct GTY(()) my_user_struct {
  int data;
  struct my_test_struct * GTY((tag("0"))) link;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  const char * GTY((skip)) string_val;
  struct my_test_struct * GTY((skip)) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
extern my_scalar_t * GTY((length("10"))) scalar_array_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) fixed_array[20];
extern struct my_test_struct * GTY((length("variable_length"))) var_array[];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) test_callback_fn)(int, struct my_test_struct *);
extern test_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_data;
  union my_test_union GTY((desc("1"))) lang_union;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container_struct {
  /* Scalar */
  my_scalar_t count;
  
  /* String */
  const char * GTY(()) description;
  
  /* Pointer */
  struct my_test_struct * GTY(()) data;
  
  /* Array within struct */
  int GTY(()) values[5];
  
  /* Union */
  union my_test_union GTY((desc("0"))) storage;
  
  /* Callback */
  test_callback_fn GTY(()) notify;
  
  /* Pointer to array */
  int * GTY((length("count"))) dynamic_array;
  
  /* Nested struct */
  struct GTY(()) nested {
    int id;
    struct container_struct * GTY((skip)) parent;
  } inner;
};

/* TYPE_UNDEFINED: This should trigger the undefined case */
/* Note: gengtype might create undefined types during parsing */
/* We'll rely on edge cases in parsing to hit this */

/* Additional variations to ensure coverage */
typedef union my_test_union * GTY(()) union_ptr_t;
typedef int GTY((length("0"))) zero_length_array[0];

/* Marked pointer with callback */
struct GTY(()) callback_container {
  void (*GTY(()) handler)(struct callback_container *);
  int state;
};

#endif /* MYTEST_GTY_H */
