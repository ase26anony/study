/* test-gtype.h - Test file for gengtype type categorization coverage */
/* This file should be placed in gcc/test-gtype/ directory */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());

/* TYPE_STRING: String pointer types */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string GTY((string));
extern const char *global_string GTY((string));

/* TYPE_STRUCT: Plain C structures */
struct my_plain_struct GTY(()) {
  int field1;
  double field2;
  my_scalar_type field3;
};

/* TYPE_USER_STRUCT: User-defined structure (from separate module/plugin) */
/* This is typically a struct defined in plugin code */
struct user_defined_struct GTY((user)) {
  int user_data;
  void *user_pointer GTY((skip));
};

/* TYPE_UNION: Union types */
union my_test_union GTY(()) {
  int int_val;
  double double_val;
  void *ptr_val;
  struct my_plain_struct *struct_ptr;
};

/* TYPE_POINTER: Pointer types with various attributes */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
struct forward_declared;
typedef struct forward_declared *forward_ptr GTY((ptr));

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("0")));
struct array_container GTY(()) {
  int count;
  int elements[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*callback_with_args)(int, const char *) GTY((callback));
struct callback_container GTY(()) {
  simple_callback cb1;
  callback_with_args cb2;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((tag("LANG_C"))) {
  int lang_data;
  void *lang_pointer;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types */
struct incomplete_struct;  /* No GTY markup - will be undefined */
union undefined_union;     /* Forward declaration without GTY */

/* Complex nested types to ensure traversal */
struct complex_container GTY(()) {
  /* Scalar */
  int scalar_field;
  
  /* String */
  const char *name GTY((string));
  
  /* Pointer */
  struct complex_container *next GTY((ptr));
  
  /* Array */
  int values[] GTY((length("array_size")));
  int array_size;
  
  /* Union */
  union {
    int as_int;
    void *as_ptr;
  } variant GTY(());
  
  /* Callback */
  simple_callback handler GTY((callback));
};

/* Another user struct for good measure */
struct another_user_struct GTY((user)) {
  struct user_defined_struct *user_link;
  my_string_type description;
};

/* Enumeration (treated as scalar by gengtype) */
typedef enum {
  VALUE_A,
  VALUE_B,
  VALUE_C
} my_enum_type GTY(());

/* Pointer to array */
typedef int (*array_pointer)[10] GTY((ptr));

/* Self-referential structure */
struct recursive_struct GTY(()) {
  int data;
  struct recursive_struct *next GTY((ptr));
  struct recursive_struct *prev GTY((ptr));
};

/* Template-like structure (C doesn't have templates, but shows complex case) */
#define DECLARE_CONTAINER(TYPE, NAME) \
  struct NAME GTY(()) { \
    TYPE *data GTY((ptr)); \
    int size; \
    TYPE elements[] GTY((length("size"))); \
  }

/* Instantiate template-like structures */
DECLARE_CONTAINER(int, int_container);
DECLARE_CONTAINER(struct my_plain_struct, struct_container);

/* Multiple indirection */
typedef struct my_plain_struct ***triple_ptr GTY((ptr));

/* Const pointer to const */
typedef const struct my_plain_struct * const const_ptr_to_const GTY((ptr));

/* Anonymous struct/union */
struct anonymous_container GTY(()) {
  struct {
    int x;
    int y;
  } point;
  union {
    int i;
    float f;
  } data;
};

/* Bitfield structure */
struct bitfield_struct GTY(()) {
  unsigned int flag1 : 1;
  unsigned int flag2 : 2;
  unsigned int flag3 : 3;
  int regular_field;
};

/* Variable length structure with callback array */
struct callback_array_struct GTY(()) {
  int num_callbacks;
  simple_callback callbacks[] GTY((length("num_callbacks")));
};

/* Mixed language struct (simulating frontend usage) */
struct mixed_lang_struct GTY((tag("MIXED"))) {
  int c_data;
  struct lang_specific_struct *lang_link GTY((ptr));
  void (*lang_callback)(void) GTY((callback));
};

/* Opaque type declarations (for TYPE_UNDEFINED) */
typedef struct completely_opaque *opaque_handle;
typedef void (*unknown_callback)(int);  /* No GTY markup */

/* Force undefined by forward declaring with GTY but not defining */
struct gty_forward_declared GTY(());

#endif /* TEST_GTYPE_H */
