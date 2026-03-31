/* test-gtype.h - Test header for gengtype type coverage */
/* This file defines various type categories to exercise the switch-case
   in gengtype.cc lines 182-213 */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef const char *const *string_array_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* This is typically a structure from client/plugin code */
struct GTY((user)) my_user_struct
{
  int user_data;
  struct my_base_struct *nested GTY((skip));
};

/* Forward declaration for TYPE_UNDEFINED test */
struct undefined_struct;

/* TYPE_UNDEFINED: Incomplete/forward declared type */
extern struct undefined_struct *undefined_ptr GTY((ptr));

/* Another undefined case - struct without GTY in this context */
struct another_undefined;

/* TYPE_UNION: Union types */
union my_union_type GTY(())
{
  int int_value;
  double float_value;
  void *pointer_value;
  my_string_type string_value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_base_struct *struct_ptr GTY((ptr));
typedef union my_union_type *union_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int variable_array[] GTY((length("my_length")));
typedef struct my_base_struct *struct_array_ptr[] GTY((ptr));

/* Flexible array member in a struct */
struct with_flex_array GTY(())
{
  int count;
  int data[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(struct my_base_struct *, my_string_type) 
  GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to identify as language-specific */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  struct my_base_struct *base_ptr;
};

/* Nested structure to test traversal */
struct container_struct GTY(())
{
  /* Scalar */
  int scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Pointer */
  struct_ptr struct_pointer;
  
  /* Array pointer */
  struct_array_ptr array_field;
  
  /* Union */
  union my_union_type union_field;
  
  /* Callback */
  simple_callback callback_field;
  
  /* Language struct */
  struct lang_specific_struct *lang_field;
  
  /* User struct */
  struct my_user_struct *user_field;
  
  /* Undefined pointer */
  opaque_ptr undefined_field;
};

/* Additional test cases for edge scenarios */

/* Pointer chain */
typedef struct container_struct **double_ptr GTY((ptr));

/* Const pointer */
typedef const struct my_base_struct *const_struct_ptr GTY((ptr));

/* Array of pointers */
typedef struct my_base_struct *pointer_array[5] GTY((ptr));

/* Self-referential structure */
struct self_ref GTY(())
{
  int value;
  struct self_ref *next GTY((ptr));
};

/* Complex nested type */
typedef struct
{
  struct container_struct container;
  union my_union_type variant;
  simple_callback handler;
} anonymous_struct GTY(());

#endif /* TEST_GTYPE_H */
