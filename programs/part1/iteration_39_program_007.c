/* Test header with GTY annotations to cover all type categories in gengtype.cc */
/* This file should be included during GCC build to trigger gengtype processing */

#ifndef GCC_MYTEST_GTY_H
#define GCC_MYTEST_GTY_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_type;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  int field1;
  my_scalar_type field2;
  struct my_test_struct *next;
};

/* TYPE_USER_STRUCT: Forward declared struct that will be user-defined */
struct my_user_struct;
typedef struct my_user_struct * GTY(()) my_user_struct_ptr;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_test_struct *struct_ptr;
};

/* TYPE_POINTER: Various pointer types with GTY annotations */
struct my_test_struct * GTY(()) global_struct_ptr;
union my_test_union * GTY(()) global_union_ptr;
int * GTY(()) int_ptr_array[10];

/* TYPE_ARRAY: Array types with GTY annotations */
extern int GTY((length("my_array_length"))) my_dynamic_array[];
extern int GTY(()) my_fixed_array[100];
extern struct my_test_struct GTY(()) struct_array[50];

/* TYPE_CALLBACK: Function pointer type with GTY annotation */
typedef void (*GTY(()) my_callback_func)(int, const char*);
extern my_callback_func GTY(()) registered_callback;

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
  tree base;
};
#endif

/* Complex nested type to ensure thorough processing */
struct GTY(()) container_struct {
  /* Scalar */
  int GTY(()) count;
  
  /* String */
  const char * GTY(()) name;
  
  /* Pointer */
  struct container_struct * GTY(()) next;
  
  /* Array */
  int GTY(()) values[20];
  
  /* Union */
  union my_test_union GTY(()) data;
  
  /* Callback */
  my_callback_func GTY(()) handler;
  
  /* Pointer to array */
  struct my_test_struct ** GTY(()) ptr_array;
  
  /* Skip annotation for edge cases */
  void * GTY((skip)) opaque_data;
};

/* Variable declarations using our GTY types */
extern struct container_struct GTY(()) *global_container;
extern union my_test_union GTY(()) global_union_var;
extern my_callback_func GTY(()) callbacks[5];

/* Template-like macro usage (common in GCC headers) */
#define DEF_GTY_STRUCT(name) \
  struct GTY(()) name { \
    int id; \
    struct name *next; \
  }

DEF_GTY_STRUCT(my_macro_struct);

/* Another pointer type with chain_next annotation */
struct GTY((chain_next("%h.next"))) chainable_struct {
  int value;
  struct chainable_struct *next;
};

#endif /* GCC_MYTEST_GTY_H */
