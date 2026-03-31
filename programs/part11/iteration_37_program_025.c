/* Test header for gengtype coverage testing */
/* This file defines various types to trigger all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String pointer type */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Plain C structure */
struct my_struct GTY(()) {
  int a;
  double b;
  my_string c;
};

/* TYPE_USER_STRUCT: User-defined structure */
/* Defined with user marker to distinguish from regular struct */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *nested GTY((skip));
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer to incomplete type */
struct forward_decl;
typedef struct forward_decl *opaque_ptr GTY((ptr));

/* Another pointer type with specific options */
typedef int *int_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
/* Fixed-size array */
typedef int fixed_array[10] GTY(());

/* Variable-length array (requires length specifier) */
struct array_container GTY(()) {
  int length;
  int elements GTY((length("%0.length")));
};

/* Zero-length array */
typedef int zero_array[] GTY((length("0")));

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_fn)(int, const char *) GTY((callback));

/* Callback with return value */
typedef int (*int_callback)(void) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
/* Using tag to mark as language-specific */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  callback_fn handler;
};

/* Another approach: structure in language-specific context */
#ifdef LANG_SPECIFIC
struct another_lang_struct GTY(()) {
  void *lang_ptr;
  int lang_id;
};
#endif

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
/* Forward declaration without GTY - will be undefined */
struct incomplete_struct;

/* Typedef to incomplete type */
typedef struct incomplete_struct incomplete_type;

/* Malformed GTY annotation (missing parentheses) */
struct bad_struct GTY {
  int x;
};

/* GTY with unknown option */
struct weird_struct GTY((unknown_option(123))) {
  int weird;
};

/* Nested structures to test traversal */
struct outer_container GTY(()) {
  struct my_struct inner GTY(());
  union my_union choice GTY(());
  callback_fn handler GTY((skip));
  int_ptr numbers GTY((length("5")));
};

/* Template-like structure for C++ (if processed) */
#ifdef __cplusplus
template<typename T>
struct template_struct GTY(()) {
  T data;
  T* next;
};
#endif

/* Enumeration type (should be scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Bitfield structure */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Self-referential structure */
struct node GTY(()) {
  int value;
  struct node *next GTY((skip));
  struct node *prev GTY((skip));
};

/* Union with pointers */
union pointer_union GTY(()) {
  struct my_struct *s_ptr;
  struct lang_struct *l_ptr;
  callback_fn func_ptr;
};

/* Array of pointers */
typedef struct my_struct *struct_ptr_array[5] GTY(());

/* Callback returning pointer */
typedef struct my_struct *(*allocator_fn)(size_t) GTY((callback));

/* String array */
typedef const char *string_array[] GTY((length("3")));

/* Complex nested type */
struct complex_type GTY(()) {
  struct {
    int x;
    int y;
  } point GTY(());
  
  union {
    int i;
    double d;
  } value GTY(());
  
  struct complex_type *children GTY((length("%0.child_count")));
  int child_count;
};

#endif /* TEST_GTYPE_H */
