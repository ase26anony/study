/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

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
  int a;
  double b;
  my_string c;
};

/* TYPE_USER_STRUCT: User-defined structure (defined in separate module) */
/* This will be defined in user-struct.h to simulate user module */

/* TYPE_UNION: Union types marked with GTY */
union my_union GTY(()) {
  int i;
  void *p GTY((skip));
  double d;
};

/* TYPE_POINTER: Pointer types with various GTY options */
typedef struct opaque_struct *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef struct my_struct *struct_ptr GTY(());

/* TYPE_ARRAY: Array types with different length specifications */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *struct_array[] GTY((length("10")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(void) GTY((callback));
typedef int (*compare_fn)(const void *, const void *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_ptr GTY((skip));
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
struct undefined_struct;  /* Forward declaration */
typedef struct incomplete *incomplete_ptr GTY(());  /* Pointer to incomplete type */

/* Malformed GTY annotation to trigger undefined categorization */
struct bad_struct {
  int x;
} /* Missing GTY annotation */;

/* Additional complex types to ensure full traversal */
struct nested_struct GTY(()) {
  struct my_struct inner GTY(());
  union my_union u GTY(());
  callback_fn callback GTY((skip));
};

/* Template-like structure with conditional fields */
struct conditional_struct GTY(()) {
  int type;
  union {
    my_scalar scalar_val;
    my_string string_val;
    struct_ptr struct_val;
  } data GTY((desc("%0.type")));
};

#endif /* TEST_GTYPE_H */
