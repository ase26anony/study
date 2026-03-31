/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *another_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(()) {
  int field1;
  my_string_type field2;
};

/* TYPE_USER_STRUCT: User-defined structure (in separate module/plugin) */
/* This is typically a struct defined in plugin code */
struct GTY((user)) my_user_struct {
  struct my_base_struct *base;
  int user_data;
};

/* TYPE_UNION: Union types */
union my_union_type GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
};

/* TYPE_POINTER: Pointer types with special handling */
typedef struct opaque_struct *opaque_ptr_type GTY((ptr));
typedef void *generic_pointer GTY((ptr));

/* Forward declaration for pointer type */
struct forward_declared;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int variable_array[] GTY((length("0")));
struct array_container GTY(()) {
  int count;
  variable_array data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific_struct {
  int lang_data;
  struct my_base_struct *related;
};

/* TYPE_UNDEFINED: Incomplete/forward declared types */
struct incomplete_struct;  /* Forward declaration without definition */
typedef struct unknown_type *unknown_ptr;  /* Pointer to unknown type */

/* Complex nested example to ensure traversal */
struct complex_container GTY(()) {
  /* Scalar */
  my_scalar_type scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Struct */
  struct my_base_struct struct_field;
  
  /* Pointer */
  opaque_ptr_type pointer_field;
  
  /* Array */
  fixed_array array_field;
  
  /* Union */
  union my_union_type union_field;
  
  /* Callback */
  simple_callback callback_field;
  
  /* Pointer to lang struct */
  struct lang_specific_struct *lang_ptr_field;
};

/* Another user struct variation */
struct GTY((user)) another_user_struct {
  struct complex_container *container;
  my_string_type name;
};

/* Enumeration (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type;

/* Template-like structure with conditional fields */
#ifdef SPECIAL_FEATURE
struct conditional_struct GTY(()) {
  int special_field;
};
#else
struct conditional_struct GTY(()) {
  int normal_field;
};
#endif

/* Self-referential structure */
struct recursive_struct GTY(()) {
  int data;
  struct recursive_struct *next GTY((skip));
};

/* Union with struct */
union mixed_union GTY(()) {
  struct my_base_struct as_struct;
  struct lang_specific_struct *as_lang_ptr;
  simple_callback as_callback;
};

#endif /* TEST_GTYPE_H */
