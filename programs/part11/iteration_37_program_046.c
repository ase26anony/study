/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file should be placed in gcc/test-gtype/ directory */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string str;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* TYPE_USER_STRUCT: Structure that will be treated as user-defined */
/* This is typically a struct defined in plugin/extension code */
struct GTY((user)) user_defined_struct {
  int user_data;
  struct my_struct *nested GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer type with special handling */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *ptr_array[] GTY((length("10")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));
typedef int (*predicate_fn)(void *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_specific_data;
  callback_fn lang_callback;
};

/* Another TYPE_STRUCT example with nested types */
struct container_struct GTY(()) {
  struct my_struct embedded;
  union my_union choice;
  struct_ptr ptr_field;
  flexible_array flex;
};

/* TYPE_UNDEFINED: Incomplete type with GTY markup */
struct incomplete_struct GTY(());
typedef struct incomplete_struct *incomplete_ptr;

/* Complex type mixing multiple categories */
struct complex_type GTY(()) {
  /* Scalar fields */
  my_scalar count;
  
  /* String field */
  my_string name;
  
  /* Pointer field */
  struct_ptr next;
  
  /* Array field */
  fixed_array data;
  
  /* Union field */
  union my_union value;
  
  /* Callback field */
  callback_fn handler;
  
  /* Nested struct */
  struct container_struct container;
};

/* Additional TYPE_POINTER variations */
typedef union my_union *union_ptr GTY((ptr));
typedef callback_fn *callback_ptr GTY((ptr));

/* TYPE_ARRAY of pointers */
typedef struct_ptr *ptr_ptr_array[] GTY((length("5")));

/* TYPE_UNION with struct member */
union mixed_union GTY(()) {
  struct my_struct as_struct;
  struct container_struct as_container;
  lang_struct as_lang;
};

/* Forward declarations that remain undefined */
struct never_defined GTY(());
typedef struct never_defined *never_ptr;

/* Enumeration type (treated as scalar) */
typedef enum {
  STATE_A,
  STATE_B,
  STATE_C
} state_enum GTY(());

/* TYPE_STRUCT with bitfields */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* TYPE_POINTER to callback */
typedef callback_fn (*meta_callback)(callback_fn) GTY((callback));

/* Self-referential structures */
struct node GTY(()) {
  int value;
  struct node *left GTY((skip));
  struct node *right GTY((skip));
};

/* TYPE_ARRAY with callback elements */
typedef callback_fn callback_array[] GTY((length("3")));

#endif /* TEST_GTYPE_H */
