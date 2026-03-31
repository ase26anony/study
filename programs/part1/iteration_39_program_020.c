/* Test header for gengtype coverage - covers all type categories in statistics collection */

#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String type */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct type */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_t field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct (forward declared then defined) */
struct user_struct;
typedef struct user_struct *user_struct_p;
struct GTY((user)) user_struct {
  int data;
  user_struct_p GTY((skip)) next;
};

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  const char * GTY((tag("0"))) string_val;
};

/* TYPE_POINTER: Various pointer types */
extern struct my_test_struct * GTY(()) global_struct_ptr;
typedef struct my_test_struct * GTY(()) struct_ptr_t;
extern struct_ptr_t GTY(()) another_ptr;

/* TYPE_ARRAY: Array types */
extern int GTY(()) int_array[10];
extern struct my_test_struct * GTY((length("5"))) struct_ptr_array[5];
extern const char * GTY((length("3"))) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) callback_func_t)(int, const char*);
extern callback_func_t GTY(()) registered_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_field;
  tree GTY((skip)) lang_tree;
};
#endif

/* TYPE_UNDEFINED: Forward declaration without definition creates undefined type */
struct GTY(()) undefined_struct;

/* Nested and complex types to ensure thorough processing */
struct GTY(()) container_struct {
  /* Multiple type categories within one struct */
  my_scalar_t scalar_field;          /* TYPE_SCALAR */
  const char * GTY(()) desc;         /* TYPE_STRING */
  struct my_test_struct * GTY(()) s; /* TYPE_POINTER to TYPE_STRUCT */
  union my_test_union GTY(()) u;     /* TYPE_UNION */
  int GTY(()) matrix[3][4];          /* TYPE_ARRAY (multi-dimensional) */
  callback_func_t GTY(()) cb;        /* TYPE_CALLBACK */
  
  /* Chain of pointers */
  struct container_struct * GTY((skip)) next;
  struct container_struct ** GTY((skip)) prev_ptr;
};

/* Template-like structure for additional coverage */
struct GTY(()) template_struct {
  int GTY(()) data;
  /* Variable length array at end */
  struct my_test_struct * GTY((length("data"))) flexible_array[];
};

#endif /* GCC_MYTEST_H */
