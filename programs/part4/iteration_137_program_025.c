/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) my_struct {
  int field1;
  my_scalar field2;
  struct undefined_type *undef_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct GTY((user)) my_user_struct {
  void *user_data;
  int user_id;
};

/* Another regular struct for counting */
struct GTY(()) another_struct {
  my_struct *next;
  int value;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
  int int_val;
  double double_val;
  my_scalar scalar_val;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
  int lang_field;
  comparison_fn compare;
};
#endif

/* Nested structure to test traversal */
struct GTY(()) container_struct {
  my_struct embedded;
  my_union choice;
  int_array numbers;
  comparison_fn sorter;
  struct undefined_type *forward_ref;
};

#endif /* TEST_STRUCTURES_H */
