/* Test header for gengtype coverage - defines all TYPE_* categories */

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
  my_scalar b;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This is typically a struct from client/plugin code */
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

/* TYPE_ARRAY: Array type with length attribute */
typedef int flexible_array[] GTY((length("array_length")));
extern int array_length;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to identify as language-specific */
struct lang_struct GTY((tag("LANG_TYPE"))) {
  int lang_data;
  callback_fn handler;
};

/* Another TYPE_STRUCT with nested types */
struct complex_struct GTY(()) {
  struct my_struct nested;
  union my_union choice;
  my_string name;
  opaque_ptr unknown;
  flexible_array flex;
  struct lang_struct *lang_ptr;
};

/* TYPE_UNDEFINED: Incomplete type definition */
/* This forward declaration will be categorized as undefined */
struct undefined_struct {
  /* No definition here - makes it undefined for gengtype */
};

/* Variable declarations using our types for reference */
extern struct my_struct global_struct;
extern struct my_user_struct *user_struct_ptr;
extern union my_union global_union;
extern callback_fn global_callback;
extern struct lang_struct lang_instance;

#endif /* TEST_GTYPE_H */
