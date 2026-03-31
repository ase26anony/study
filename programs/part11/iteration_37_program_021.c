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

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct GTY(());

/* TYPE_USER_STRUCT: User-defined structure (simulated via special marker) */
struct user_defined GTY((user)) {
  int user_data;
  struct my_struct *nested;
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
  int i;
  double d;
  void *p;
  my_string s;
};

/* TYPE_POINTER: Pointer types with various qualifiers */
typedef struct my_struct *struct_ptr GTY((ptr));
typedef const struct my_struct *const_struct_ptr GTY((ptr));
typedef void *generic_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
typedef struct my_struct struct_array[] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*complex_callback)(int, const char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific GTY((tag("LANG"))) {
  int lang_data;
  union my_union lang_union;
};

/* Additional complex types to ensure full traversal */

/* Nested structure with pointer */
struct container GTY(()) {
  struct my_struct embedded;
  struct my_struct *pointer;
  union my_union data;
  simple_callback cb;
};

/* Self-referential structure */
struct tree_node GTY(()) {
  int value;
  struct tree_node *left GTY((skip));
  struct tree_node *right GTY((skip));
};

/* Union containing pointers */
union pointer_union GTY(()) {
  struct my_struct *s_ptr;
  struct container *c_ptr;
  generic_ptr g_ptr;
};

/* Array of pointers */
typedef struct my_struct *ptr_array[] GTY((length("0")));

/* Callback with arguments */
typedef void (*event_handler)(int event_id, void *data) GTY((callback));

/* String array */
typedef const char *string_array[] GTY((string));

/* Mixed structure for comprehensive testing */
struct comprehensive GTY(()) {
  /* Scalars */
  my_scalar scalar1;
  double scalar2 GTY(());
  
  /* Strings */
  my_string str1;
  char *str2 GTY((string));
  
  /* Pointers */
  struct_ptr sptr;
  generic_ptr gptr;
  
  /* Arrays */
  fixed_array fixed;
  variable_array *varray;
  
  /* Callback */
  event_handler handler;
  
  /* Union */
  union my_union data_union;
  
  /* Nested structures */
  struct container nested;
  struct lang_specific *lang_ptr;
  
  /* Self-reference */
  struct comprehensive *next GTY((skip));
};

/* Incomplete type for TYPE_UNDEFINED */
struct forward_declared GTY(());

/* Another undefined case - type with problematic GTY annotation */
struct problematic GTY((invalid_option));

#endif /* TEST_GTYPE_H */
