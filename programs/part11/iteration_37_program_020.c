/* Test header file for gengtype coverage testing
   This file defines various types to trigger all categories in the type counter */

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
/* This typically requires being in a separate module or using special options */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *link GTY((skip));
};

/* TYPE_UNION: Union type marked with GTY */
union my_union GTY(()) {
  int i;
  void *p;
  my_scalar s;
};

/* TYPE_POINTER: Pointer to incomplete/opaque type */
struct opaque_type;
typedef struct opaque_type *opaque_ptr GTY((ptr));

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* Another TYPE_STRUCT with nested types */
struct complex_struct GTY(()) {
  struct my_struct nested;
  union my_union choice;
  opaque_ptr ptr;
  fixed_array arr;
  my_string str;
};

/* TYPE_UNDEFINED: Incomplete type without definition */
/* This forward declaration will be counted as undefined */
struct incomplete_type GTY(());

/* More pointer types for coverage */
typedef struct complex_struct *complex_ptr GTY((ptr));
typedef union my_union *union_ptr GTY((ptr));

/* Array of pointers */
typedef struct my_struct *struct_ptr_array[] GTY((length("0")));

/* Callback with complex signature */
typedef int (*complex_callback)(struct complex_struct *, opaque_ptr) 
  GTY((callback));

/* Language struct with callback */
struct GTY((tag("LANG"))) lang_struct_with_cb {
  complex_callback cb;
  struct lang_struct *next;
};

/* Union containing various types */
union variant_union GTY(()) {
  struct my_struct s;
  struct complex_struct cs;
  opaque_ptr p;
  callback_fn fn;
};

/* Self-referential structure */
struct recursive_struct GTY(()) {
  int value;
  struct recursive_struct *next GTY((skip));
};

/* Structure with array member */
struct with_array GTY(()) {
  int count;
  int items[1] GTY((length("%0.count")));
};

#endif /* TEST_GTYPE_H */
