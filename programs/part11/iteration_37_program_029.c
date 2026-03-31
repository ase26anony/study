/* test-gtype.h - Comprehensive test file for gengtype type categorization */
/* This file should be added to GTFILES in the GCC build system */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const constant_string GTY((string));

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

/* TYPE_USER_STRUCT: Structures with user-defined markers */
/* These are typically from plugin or external code */
#define USER_GTY_MARKER GTY((user))

struct user_defined_struct USER_GTY_MARKER {
  int user_data;
  void *user_ptr GTY((skip));
};

/* Alternative approach: structure defined with special callback option */
struct callback_user_struct GTY((user)) {
  int (*user_func)(void) GTY((skip));
  int user_field;
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p GTY((skip));
  const char *str GTY((string));
};

union tagged_union GTY((desc("(%1.type == 0) ? &int_type : &string_type"))) {
  int type;
  int int_value;
  const char *string_value GTY((string));
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct opaque_struct *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef const struct my_struct *const_struct_ptr GTY((ptr));

/* Forward declaration for opaque pointer */
struct opaque_struct;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *struct_ptr_array[] GTY((length("10")));

/* Variable length array in a structure */
struct with_array GTY(()) {
  int count;
  int data[] GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*callback_with_args)(int, const char *) GTY((callback));
typedef void (*nested_callback)(simple_callback) GTY((callback));

/* Structure containing callbacks */
struct callback_container GTY(()) {
  simple_callback cb1;
  callback_with_args cb2;
  void (*inline_cb)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* These are typically identified by special tags or locations */

/* Approach 1: Using tag option */
struct lang_specific_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_private GTY((skip));
};

/* Approach 2: Structure with language-specific marker */
#define LANG_GTY GTY((tag("LANG_SPECIFIC")))

struct cplusplus_struct LANG_GTY {
  int cpp_data;
  struct lang_specific_struct *next LANG_GTY;
};

/* TYPE_UNDEFINED: Incomplete/forward declarations and malformed types */

/* Forward declaration without definition - will be TYPE_UNDEFINED */
struct undefined_struct GTY(());

/* Malformed GTY annotation */
struct bad_struct {
  int x;
} /* Missing GTY annotation */;

/* Typedef with incomplete type */
typedef struct incomplete *incomplete_ptr GTY(());

/* Self-referential incomplete type */
struct self_ref GTY(()) {
  struct self_ref *next;
  /* Missing GTY on next causes issues */
};

/* Complex nested type to test traversal */
struct container GTY(()) {
  /* Mix of all types */
  my_scalar scalar_field;
  my_string string_field;
  struct my_struct struct_field;
  union my_union union_field;
  opaque_ptr pointer_field;
  fixed_array array_field;
  simple_callback callback_field;
  struct lang_specific_struct lang_field;
  
  /* Nested container */
  struct container *nested GTY((skip));
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Template-like structure for C++ (if processed) */
#ifdef __cplusplus
template<typename T>
struct template_struct GTY(()) {
  T data;
  template_struct<T> *next GTY((skip));
};
#endif

/* Macro-defined type variations */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct name##_struct GTY(()) { \
    field_type field; \
    struct name##_struct *next GTY((skip)); \
  }

DEFINE_GTY_STRUCT(macro_int, int);
DEFINE_GTY_STRUCT(macro_ptr, void*);

/* Test variable declarations with GTY */
extern struct my_struct global_struct GTY(());
extern const char *global_string GTY((string));
extern int global_array[] GTY((length("100")));

#endif /* TEST_GTYPE_H */
