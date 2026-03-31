/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef double my_double GTY(());
typedef unsigned long my_ulong GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));

/* TYPE_STRUCT: Plain C structures */
struct my_struct GTY(()) {
  int field1;
  double field2;
  my_string str_field;
};

/* TYPE_USER_STRUCT: User-defined structure (in separate module/plugin context) */
/* This will be recognized as user struct when processed from plugin */
struct GTY((user)) user_defined_struct {
  int user_data;
  struct my_struct *nested GTY((skip));
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  my_string str_val;
};

/* TYPE_POINTER: Pointer types */
typedef struct opaque_type *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef struct my_struct *struct_ptr GTY(());

/* Forward declaration for pointer usage */
struct forward_declared GTY(());

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct struct_array[5] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));
typedef void (*struct_callback)(struct my_struct *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_specific_data;
  union myunion *lang_union GTY((ptr));
};

/* TYPE_UNDEFINED: Incomplete/malformed types */
/* Forward declaration without definition */
struct undefined_struct GTY(());
/* Malformed GTY annotation */
struct malformed_struct GTY((invalid_option));

/* Complex nested example to ensure traversal */
struct container GTY(()) {
  /* Scalar */
  int count GTY(());
  
  /* String */
  my_string name GTY((string));
  
  /* Struct */
  struct my_struct data GTY(());
  
  /* Pointer */
  opaque_ptr unknown GTY((ptr));
  
  /* Array */
  flexible_array items GTY((length("count")));
  
  /* Union */
  union myunion value GTY(());
  
  /* Callback */
  simple_callback handler GTY((callback));
  
  /* Nested pointer to lang struct */
  struct lang_struct *lang_data GTY((ptr));
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Template-like structure for C++ mode */
#ifdef __cplusplus
template<typename T>
struct GTY(()) template_struct {
  T data;
  template_struct<T> *next GTY((skip));
};
#endif

#endif /* TEST_GTYPE_H */
