/* Test header for gengtype coverage - covers all type categories in statistics */

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

/* TYPE_USER_STRUCT: Forward declared struct that will be defined elsewhere */
struct user_defined_struct;
typedef struct user_defined_struct GTY(()) *user_struct_ptr;

/* TYPE_UNION: Union type */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  void * GTY((tag("0"))) ptr_val;  /* tag annotation for union discrimination */
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
union my_test_union * GTY(()) my_union_pointer;

/* TYPE_ARRAY: Array types */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
extern my_callback_fn GTY(()) current_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_test_struct {
  int lang_specific_field;
  tree GTY((length("0"))) lang_tree_field;  /* tree is GCC internal type */
};
#endif

/* Nested/complex types to ensure thorough processing */
struct GTY(()) container_struct {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  struct my_test_struct * GTY(()) nested_struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
  int GTY(()) matrix[3][3];           /* TYPE_ARRAY (multi-dimensional) */
  union my_test_union data_union;     /* TYPE_UNION */
  my_callback_fn callback;            /* TYPE_CALLBACK */
};

/* Variable declarations using our types */
extern struct container_struct GTY(()) global_container;
extern my_scalar_t GTY(()) global_scalars[20];

/* Function prototypes that might use GTY types */
extern void init_test_types(void);
extern void register_callback(my_callback_fn GTY(()) cb);

#endif /* GCC_MYTEST_H */
