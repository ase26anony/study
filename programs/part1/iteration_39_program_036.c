/* Test header for gengtype coverage - covers all type categories in statistics */
#ifndef GCC_MYTEST_H
#define GCC_MYTEST_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_SCALAR: Basic scalar type with GTY annotation */
typedef int GTY(()) my_scalar_t;

/* TYPE_STRING: String pointer with GTY annotation */
extern const char * GTY(()) my_test_string;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_test_struct {
  my_scalar_t field1;
  int field2;
  const char * GTY((skip)) name;  /* skip annotation for coverage */
};

/* TYPE_USER_STRUCT: User-defined struct type */
typedef struct my_test_struct GTY(()) my_user_struct_t;

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_test_union {
  int int_val;
  double double_val;
  struct my_test_struct * GTY((tag("0"))) struct_ptr;
};

/* TYPE_POINTER: Various pointer types */
struct my_test_struct * GTY(()) my_struct_pointer;
union my_test_union * GTY(()) my_union_pointer;
my_scalar_t * GTY(()) my_scalar_pointer;

/* TYPE_ARRAY: Array types with GTY annotation */
extern int GTY(()) my_int_array[10];
extern struct my_test_struct GTY(()) my_struct_array[5];
extern const char * GTY((length("strlen(%h)"))) string_array[20];

/* TYPE_CALLBACK: Function pointer (callback) type */
typedef void (*GTY(()) my_callback_fn)(int, const char*);
typedef int (*GTY(()) compare_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_decl {
  int lang_specific;
  tree GTY((skip)) chain;
};
#endif

/* Complex nested type for additional coverage */
struct GTY(()) complex_container {
  /* Contains multiple type categories */
  my_scalar_t scalar_field;           /* TYPE_SCALAR */
  struct my_test_struct *struct_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
  union my_test_union data;           /* TYPE_UNION */
  my_callback_fn callback;            /* TYPE_CALLBACK */
  int GTY(()) dynamic_array[0];       /* TYPE_ARRAY (flexible array member) */
};

/* Variable declarations using the types */
extern struct complex_container GTY(()) *global_container;
extern my_user_struct_t GTY(()) user_struct_instance;
extern compare_fn GTY(()) sort_comparator;

/* Template-like structure for C++ mode coverage */
#ifdef __cplusplus
template<typename T>
struct GTY(()) template_wrapper {
  T GTY(()) value;
  T* GTY(()) pointer;
};
#endif

#endif /* GCC_MYTEST_H */
