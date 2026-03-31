/* Test header for gengtype coverage - defines all TYPE_* categories */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

/* Include GCC's gtype header for GTY macro */
#include "gtype.h"

/* TYPE_UNDEFINED: Forward declaration without GTY or malformed GTY */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Malformed GTY annotation to trigger TYPE_UNDEFINED */
struct malformed_gt_struct GTY;  /* Missing parentheses */

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar GTY(());
typedef double my_double GTY(());
typedef unsigned long my_ulong GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const const_string_ptr GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_struct GTY(()) {
  int field1;
  double field2;
  my_scalar field3;
};

struct nested_struct GTY(()) {
  struct my_struct inner GTY(());
  int data;
};

/* TYPE_USER_STRUCT: Structure with user-defined marker */
/* This typically requires being in a separate module or using special options */
struct user_defined_struct GTY((user)) {
  int user_data;
  void *user_ptr GTY((skip));
};

/* Alternative approach: Define in a way that gengtype treats as user struct */
#ifdef IN_USER_HEADER
struct another_user_struct GTY(()) {
  long user_field;
};
#endif

/* TYPE_UNION: Union types marked with GTY */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

union tagged_union GTY(()) {
  struct {
    int type;
  } header;
  struct {
    int type;
    int value;
  } integer;
  struct {
    int type;
    double value;
  } floating;
};

/* TYPE_POINTER: Pointer types with various GTY options */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));
typedef union my_union *union_ptr GTY((ptr));

/* Opaque pointer to incomplete type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int int_array[10] GTY(());
typedef struct my_struct struct_array[5] GTY(());

/* Variable length array - requires length specifier */
typedef int flexible_array[] GTY((length("my_struct::field1")));

/* Array of pointers */
typedef struct my_struct *ptr_array[20] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*int_callback)(int, double) GTY((callback));
typedef void (*struct_callback)(struct my_struct *) GTY((callback));

/* Callback with user data */
typedef void (*user_callback)(void *user_data) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure types */
/* Typically identified by tag or being in language-specific dir */
struct lang_specific_struct GTY((tag("LANG"))) {
  int lang_data;
  void *lang_private;
};

/* Another approach: Use language-specific marker in comments */
/* Language: C++ */
struct cpp_lang_struct GTY(()) {
  int cpp_data;
  /* GC track this as language-specific */
};

/* Complex type combinations to ensure thorough traversal */
struct complex_container GTY(()) {
  /* Mix of different types */
  my_scalar scalar_field;
  my_string string_field GTY((string));
  struct my_struct struct_field GTY(());
  union my_union union_field GTY(());
  struct_ptr pointer_field GTY((ptr));
  int_array array_field;
  simple_callback callback_field GTY((callback));
  
  /* Nested anonymous union */
  union GTY(()) {
    int as_int;
    double as_double;
  } anonymous_union;
  
  /* Flexible array member */
  int flexible_member[] GTY((length("0")));
};

/* Template-like structure (for C++ mode) */
#ifdef __cplusplus
template<typename T>
struct template_struct GTY(()) {
  T data;
  template_struct<T> *next GTY((ptr));
};
#endif

/* Enumeration type (should be treated as scalar) */
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
  unsigned int padding : 26;
};

/* Self-referential structure */
struct linked_node GTY(()) {
  int data;
  struct linked_node *next GTY((ptr));
  struct linked_node *prev GTY((ptr));
};

/* Union containing pointers */
union pointer_union GTY(()) {
  int *int_ptr;
  struct my_struct *struct_ptr;
  void **void_ptr_ptr;
};

#endif /* TEST_GTYPE_H */
