/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar_type GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string_type GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_base_struct GTY(())
{
  int field1;
  my_scalar_type field2;
  my_string_type field3;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This typically requires being in a separate module/plugin */
struct GTY((user)) my_user_struct
{
  struct my_base_struct *base GTY((skip));
  void *user_data;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union_type GTY(())
{
  int int_val;
  double double_val;
  struct my_base_struct *struct_ptr;
  my_string_type string_ptr;
};

/* TYPE_POINTER: Pointer to opaque/incomplete type */
struct forward_declared;
typedef struct forward_declared *opaque_pointer_type GTY((ptr));

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array_type[] GTY((length("array_length")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_function_type)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific_struct
{
  int lang_data;
  callback_function_type lang_callback;
};

/* TYPE_UNDEFINED: Forward declaration without complete definition */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_pointer;

/* Composite structure that references many types */
struct composite_container GTY(())
{
  /* Scalar */
  my_scalar_type scalar_field;
  
  /* String */
  my_string_type string_field;
  
  /* Struct pointer */
  struct my_base_struct *struct_ptr_field;
  
  /* User struct */
  struct my_user_struct *user_struct_field;
  
  /* Union */
  union my_union_type union_field;
  
  /* Opaque pointer */
  opaque_pointer_type opaque_ptr_field;
  
  /* Array (as pointer) */
  flexible_array_type *array_ptr_field;
  
  /* Callback */
  callback_function_type callback_field;
  
  /* Language struct */
  struct lang_specific_struct *lang_struct_field;
  
  /* Undefined pointer */
  undefined_pointer undefined_field;
  
  /* Self-reference for testing */
  struct composite_container *next GTY((skip));
};

/* Variable length array structure */
struct var_array_struct GTY(())
{
  int array_length;
  int data[] GTY((length("array_length")));
};

/* Nested structure for additional coverage */
struct nested_container GTY(())
{
  struct composite_container main;
  struct var_array_struct *var_array;
};

/* Enumeration type (treated as scalar by gengtype) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Function declarations that use these types */
void process_callback(callback_function_type cb) GTY((extern));
struct composite_container *create_container(void) GTY((malloc));

#endif /* TEST_GTYPE_H */
