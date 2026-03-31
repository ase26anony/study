/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing
 * This file should be added to GTFILES in the GCC build system
 */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype.h"

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
  my_string name;
};

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This typically requires being in a separate module or plugin */
struct GTY((user)) my_user_struct {
  int user_data;
  struct my_struct *nested GTY((skip));
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer types with various annotations */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef union my_union *union_ptr GTY((ptr));

/* Opaque pointer for TYPE_POINTER */
struct opaque_type;
typedef struct opaque_type *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int flexible_array[] GTY((length("0")));
typedef struct my_struct *struct_array[] GTY((length("10")));

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));
typedef void (*struct_callback)(struct my_struct *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_struct {
  int lang_data;
  simple_callback handler;
  struct my_struct *base_struct;
};

/* Nested structure to ensure traversal */
struct container_struct GTY(()) {
  /* TYPE_SCALAR */
  int scalar_field GTY(());
  
  /* TYPE_STRING */
  const char *string_field GTY((string));
  
  /* TYPE_POINTER */
  struct my_struct *struct_ptr_field GTY((ptr));
  
  /* TYPE_ARRAY */
  int array_field[5] GTY(());
  
  /* TYPE_CALLBACK */
  void (*callback_field)(void) GTY((callback));
  
  /* TYPE_UNION */
  union my_union union_field GTY(());
  
  /* Reference to TYPE_LANG_STRUCT */
  struct lang_struct *lang_field GTY((ptr));
};

/* Another structure for TYPE_USER_STRUCT differentiation */
struct GTY((user, desc("1"))) another_user_struct {
  int id;
  struct container_struct *container GTY((ptr));
};

/* Incomplete type that should remain TYPE_UNDEFINED */
struct incomplete_struct GTY(());

/* Template-like structure for edge cases */
struct GTY((for_user)) user_template {
  int template_data;
  struct my_user_struct *user_ref GTY((skip));
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum GTY(());

/* Complex nested type definitions */
typedef struct GTY(()) nested_container {
  struct my_struct inner_struct;
  union my_union inner_union;
  my_string strings[3] GTY((length("3")));
  struct nested_container *next GTY((ptr));
} nested_container;

/* Callback with complex signature */
typedef void (*error_callback)(int errno, const char *message, void *user_data) 
  GTY((callback));

/* Array of pointers */
typedef struct my_struct *struct_ptr_array[] GTY((length("dynamic")));

/* Union with nested structure */
union GTY(()) complex_union {
  struct {
    int type;
    void *data GTY((ptr));
  } s;
  long long int big_int;
  double precise_float;
};

/* Marked pointer with special handling */
typedef struct my_struct * GTY((reorder("skip"))) skipped_ptr;

/* Variable length structure */
struct GTY(()) var_len_struct {
  int length;
  int data[] GTY((length("length")));
};

/* Bitfield structure */
struct GTY(()) bitfield_struct {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Self-referential structure */
struct GTY(()) self_ref {
  int data;
  struct self_ref *next GTY((ptr));
  struct self_ref *prev GTY((ptr));
};

/* Multiple indirection */
typedef struct my_struct **double_ptr GTY((ptr));
typedef struct my_struct ***triple_ptr GTY((ptr));

/* Const-qualified pointers */
typedef const struct my_struct *const_struct_ptr GTY((ptr));
typedef struct my_struct *const const_ptr_to_struct GTY((ptr));

/* Anonymous union in structure */
struct GTY(()) with_anonymous_union {
  int type;
  union {
    int int_value;
    double double_value;
    void *ptr_value GTY((ptr));
  } value;
};

/* Structure with callback array */
struct GTY(()) with_callbacks {
  int count;
  simple_callback handlers[] GTY((length("count")));
};

#endif /* TEST_GTYPE_H */
