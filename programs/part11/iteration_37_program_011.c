/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing */

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
typedef const char *const const_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string str;
};

struct another_struct GTY(()) {
  struct my_struct *next;
  int count;
};

/* TYPE_USER_STRUCT: Structures with user-defined markers */
/* These are typically from external/plugin code */
#define USER_GTY_MARKER GTY((user))

struct user_defined_struct USER_GTY_MARKER {
  int user_data;
  void *user_ptr;
};

/* Alternative: Structure defined with special callback option */
struct callback_user_struct GTY((user)) {
  int (*compare)(const void *, const void *);
  void *data;
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

union tagged_union GTY(()) {
  struct {
    int type;
    union my_union data;
  } tagged;
  long long raw;
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct incomplete *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef const struct my_struct *const_struct_ptr GTY((ptr));

/* Forward declaration for pointer testing */
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct struct_array[] GTY((length("sizeof(struct my_struct)")));

/* Variable length array in struct */
struct with_array GTY(()) {
  int count;
  int items GTY((length("%0.count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*comparison_fn)(const void *, const void *) GTY((callback));
typedef void (*event_handler)(int event, void *data) GTY((callback));

/* Callback in struct */
struct with_callbacks GTY(()) {
  simple_callback init;
  comparison_fn compare;
  event_handler on_event;
};

/* TYPE_LANG_STRUCT: Language-specific structure types */
struct lang_struct GTY((tag("LANG_CPLUSPLUS"))) {
  int cpp_specific;
  void *vtbl;
};

struct java_lang_struct GTY((tag("LANG_JAVA"))) {
  int java_magic;
  struct lang_struct *base;
};

/* TYPE_UNDEFINED: Incomplete/forward declared types */
/* These should be categorized as undefined */
struct undefined_struct;
union undefined_union;

/* Malformed GTY annotation */
struct bad_annotation {
  int x;
}; /* Missing GTY(()) */

/* Pointer to undefined type */
typedef struct undefined_struct *ptr_to_undefined GTY((ptr));

/* Nested complex type for comprehensive testing */
struct container GTY(()) {
  /* Scalar */
  my_scalar id;
  
  /* String */
  my_string name;
  
  /* Struct */
  struct my_struct data;
  
  /* User struct */
  struct user_defined_struct *user_data;
  
  /* Union */
  union my_union variant;
  
  /* Pointer */
  opaque_ptr opaque;
  
  /* Array */
  flexible_array flex;
  
  /* Callback */
  simple_callback cleanup;
  
  /* Lang struct */
  struct lang_struct *lang_specific;
  
  /* Pointer to undefined */
  ptr_to_undefined unknown;
};

/* Template-like macro for generating multiple instances */
#define DEFINE_GTY_STRUCT(name, field_type) \
  struct gty_struct_##name GTY(()) { \
    field_type field; \
    struct gty_struct_##name *next; \
  }

DEFINE_GTY_STRUCT(int, int);
DEFINE_GTY_STRUCT(ptr, void*);

/* Self-referential structure */
struct recursive GTY(()) {
  int value;
  struct recursive *next;
  struct recursive *prev;
};

/* Union with struct */
union complex_union GTY(()) {
  struct {
    int type;
    union {
      int i;
      double d;
      my_string s;
    } value;
  } tagged;
  long long raw[2];
};

#endif /* TEST_GTYPE_H */
