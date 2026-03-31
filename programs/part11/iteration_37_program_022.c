/* Test header for gengtype coverage - defines all TYPE_* categories */

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
typedef const char *const const_string_ptr GTY((string));

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

/* TYPE_USER_STRUCT: User-defined structure (from separate module/plugin) */
/* This will be treated as user struct when included from plugin context */
struct user_defined GTY((user)) {
  int user_id;
  void *user_data GTY((ptr));
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p GTY((ptr));
  const char *s GTY((string));
};

union tagged_union GTY((desc("tag_field"))) {
  int tag_field;
  struct {
    int type;
    union my_union data;
  } variant;
};

/* TYPE_POINTER: Pointer types */
typedef struct opaque *opaque_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef struct my_struct *struct_ptr GTY((ptr));

/* Forward declaration for pointer to incomplete type */
struct incomplete;
typedef struct incomplete *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("array_length")));
typedef const char *string_array[] GTY((length("str_count")));

struct with_array GTY(()) {
  int array_length;
  flexible_array data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_fn)(int, const char *) GTY((callback));
typedef int (*compare_fn)(const void *, const void *) GTY((callback));
typedef void (*simple_callback)(void) GTY((callback));

struct with_callback GTY(()) {
  callback_fn handler;
  void *context GTY((ptr));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_struct GTY((tag("LANG_TYPE"))) {
  int lang_specific;
  void *lang_data GTY((ptr));
};

struct cpp_macro GTY((tag("CPP"))) {
  const char *name GTY((string));
  int line;
};

/* TYPE_UNDEFINED: Incomplete/undefined types */
/* Forward declaration without definition */
struct undefined_type;
typedef struct undefined_type *undefined_ptr;

/* Malformed GTY annotation */
struct bad_struct GTY(() {  /* Missing closing parenthesis */
  int x;
};

/* Multiple categories in one structure to ensure all counters increment */
struct comprehensive GTY(()) {
  /* Contains scalar */
  my_scalar scalar_field;
  
  /* Contains string */
  my_string string_field;
  
  /* Contains pointer */
  struct_ptr pointer_field;
  
  /* Contains array */
  fixed_array array_field;
  
  /* Contains callback */
  callback_fn callback_field;
  
  /* Contains union */
  union my_union union_field;
  
  /* Nested struct */
  struct nested GTY(()) {
    int nested_data;
  } nested_field;
};

/* Template for generating multiple instances */
#define DECLARE_STRUCT(name, field) \
  struct name##_struct GTY(()) { \
    int id; \
    field value; \
  }

DECLARE_STRUCT(scalar_wrapper, my_scalar);
DECLARE_STRUCT(string_wrapper, my_string);
DECLARE_STRUCT(pointer_wrapper, struct_ptr);

/* Ensure all type categories are referenced */
extern struct my_struct global_struct;
extern union my_union global_union;
extern opaque_ptr global_opaque;
extern callback_fn global_callback;
extern struct lang_struct global_lang_struct;

#endif /* TEST_GTYPE_H */
