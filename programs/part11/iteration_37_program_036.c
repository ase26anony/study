/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string str;
};

struct another_struct GTY(()) {
  struct my_struct *next GTY((skip));
  int data;
};

/* TYPE_USER_STRUCT: User-defined structure type */
/* This is typically a structure from plugin/extension code */
#define USER_STRUCT_MARKER
struct user_defined_struct GTY((user)) {
  int user_data;
  void *user_ptr GTY((ptr));
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p GTY((ptr));
  const char *str GTY((string));
};

union tagged_union GTY((desc("tag_field"))) {
  int tag_field;
  struct {
    int type;
    union my_union data;
  } nested;
};

/* TYPE_POINTER: Pointer types */
typedef struct opaque_struct *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef const struct my_struct *const_struct_ptr GTY((ptr));

/* Forward declaration for pointer type */
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int int_array[10] GTY(());
typedef struct my_struct struct_array[5] GTY(());
typedef int flexible_array[] GTY((length("0")));

struct with_array GTY(()) {
  int count;
  int elements[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*callback_with_args)(int, const char *) GTY((callback));
typedef void (*callback_returning_ptr)(struct my_struct **) GTY((callback));

struct with_callback GTY(()) {
  simple_callback cb;
  int priority;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct lang_specific_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_private GTY((skip));
};

struct cplusplus_struct GTY((tag("CPLUSPLUS"))) {
  int vtable_offset;
  struct lang_specific_struct *base GTY((ptr));
};

/* TYPE_UNDEFINED: Incomplete/malformed types */
/* Forward declaration without definition */
struct undefined_struct;
typedef struct undefined_struct undefined_type;

/* Malformed GTY annotation */
struct malformed GTY((invalid_option)) {
  int x;
};

/* Incomplete array without length specifier */
struct incomplete_array GTY(()) {
  int data[];
};

/* Nested problematic type */
typedef struct {
  int x;
  /* Missing GTY on pointer field */
  struct undefined_struct *bad_ptr;
} problematic_type;

#endif /* TEST_GTYPE_H */
