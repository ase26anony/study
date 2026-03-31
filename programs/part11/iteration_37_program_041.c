/* test-gtype.h - Test file for gengtype type categorization coverage */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure marked for GC */
struct my_struct GTY(()) {
  int a;
  my_scalar b;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This typically requires being in a separate module/plugin */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link;
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p;
  my_scalar s;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* TYPE_POINTER: Pointer to incomplete type */
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array[] GTY((length("0")));

/* Another array type */
struct array_container GTY(()) {
  int count;
  int items[10] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* TYPE_UNDEFINED: Incomplete type with GTY markup */
/* This should trigger TYPE_UNDEFINED */
struct GTY(()) incomplete_type;

/* Another undefined case: forward declared struct used in typedef */
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY(());

/* Complex nested structure to test multiple categories */
struct container GTY(()) {
  my_scalar scalar_field;
  my_string string_field;
  struct my_struct *struct_ptr GTY((skip));
  union my_union union_field;
  callback_fn callback_field;
  struct lang_struct *lang_ptr;
  opaque_ptr undefined_ptr;
  struct array_container array_field;
};

/* Enum type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Bitmask type */
typedef unsigned int bitmask GTY(());

/* Nested pointer structure */
struct node GTY(()) {
  int value;
  struct node *next GTY((skip));
  struct node *prev GTY((skip));
};

/* Variable length structure */
struct var_struct GTY(()) {
  int length;
  int data[1] GTY((length("length")));
};

/* Union with pointers */
union ptr_union GTY(()) {
  void *ptr;
  long long intval;
  double floatval;
};

#endif /* TEST_GTYPE_H */
