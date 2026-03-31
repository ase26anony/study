/* Test header for gengtype coverage - defines all TYPE_* categories */

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
/* This is typically a struct from plugin/extension code */
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

/* TYPE_POINTER: Pointer to incomplete/undefined type */
typedef struct undefined_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array type with length specifier */
typedef int flexible_array[] GTY((length("0")));

/* Another array type */
struct array_container GTY(()) {
  int count;
  int values[10] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* TYPE_UNDEFINED: Incomplete type definition */
/* This forward declaration without complete definition should be undefined */
struct undefined_struct GTY(());

/* Another undefined case: malformed GTY annotation */
struct malformed_struct {
  int x;
} /* Missing GTY annotation - will be undefined for gengtype */;

/* Complex nested types to ensure full traversal */
struct complex_container GTY(()) {
  /* Nested scalar */
  my_scalar scalar_field;
  
  /* Nested string */
  my_string string_field;
  
  /* Nested struct */
  struct my_struct struct_field;
  
  /* Nested user struct */
  struct my_user_struct *user_struct_field GTY((skip));
  
  /* Nested union */
  union my_union union_field;
  
  /* Nested pointer */
  opaque_ptr pointer_field;
  
  /* Nested array */
  flexible_array *array_field;
  
  /* Nested callback */
  callback_fn callback_field;
  
  /* Nested lang struct */
  struct lang_struct *lang_struct_field;
};

/* Enum type (treated as scalar by gengtype) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Function pointer in struct */
struct callback_container GTY(()) {
  const char *name;
  void (*func)(void) GTY((callback));
};

/* Variable length array in struct */
struct vla_container GTY(()) {
  int len;
  char data[1] GTY((length("len")));
};

/* Chain of pointers for depth testing */
struct pointer_chain GTY(()) {
  struct pointer_chain *next GTY((ptr));
  int value;
};

/* Self-referential structure */
struct self_ref GTY(()) {
  int data;
  struct self_ref *next;
};

/* Union with pointers */
union pointer_union GTY(()) {
  struct my_struct *s_ptr;
  struct my_user_struct *u_ptr;
  opaque_ptr o_ptr;
};

#endif /* TEST_GTYPE_H */
