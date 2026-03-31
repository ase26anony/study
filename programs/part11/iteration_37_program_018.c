/* test-gtype.h - Comprehensive test file for gengtype type categorization */
/* This file should be placed in gcc/test-gtype/test-gtype.h */

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

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int field1;
  double field2;
  my_string field3;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef union my_union *union_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *struct_array[] GTY((length("0")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_ptr;
};

/* Nested structures to test complex type graphs */
struct container GTY(()) {
  struct my_struct *nested GTY((ptr));
  union my_union data;
  flexible_array flex;
  simple_callback cb;
};

/* For TYPE_USER_STRUCT - this will be considered a user struct
   when included from plugin/separate compilation unit */
struct user_defined GTY(()) {
  int user_field;
  struct container *cont GTY((ptr));
};

/* Incomplete type for TYPE_UNDEFINED */
struct incomplete GTY(());

/* Another undefined case - type with malformed GTY annotation */
struct problematic GTY((unknown_tag));

/* Array of pointers */
typedef struct my_struct *ptr_array[] GTY((length("0")));

/* Union containing pointers */
union pointer_union GTY(()) {
  struct my_struct *s_ptr;
  union my_union *u_ptr;
  generic_ptr g_ptr;
};

/* Callback with arguments */
typedef void (*event_callback)(int event_id, void *data) GTY((callback));

/* String array */
typedef const char *string_array[] GTY((length("0")));

/* Self-referential structure */
struct recursive GTY(()) {
  int value;
  struct recursive *next GTY((ptr));
};

/* Mixed type structure */
struct mixed GTY(()) {
  my_scalar scalar;
  my_string str;
  struct my_struct nested;
  union my_union uni;
  struct my_struct *ptr GTY((ptr));
  flexible_array arr;
  simple_callback cb;
};

#endif /* TEST_GTYPE_H */
